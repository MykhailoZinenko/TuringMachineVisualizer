#include "TapeDocument.h"
#include "../model/Tape.h"
#include "../model/TuringMachine.h"
#include "../core/SessionManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

TapeDocument::TapeDocument(const std::string& name, const std::string& filePath)
    : Document(DocumentType::TAPE, name, filePath),
      m_initialHeadPosition(0)
{
    // Create a new empty tape
    m_tape = std::make_unique<Tape>();

    // Connect to SessionManager for machine updates
    connect(&SessionManager::getInstance(), &SessionManager::activeMachineUpdated,
            this, &TapeDocument::onActiveMachineUpdated);
}

TapeDocument::~TapeDocument()
{
}

void TapeDocument::setInitialContent(const std::string& content)
{
    m_initialContent = content;
    m_tape->setInitialContent(content);

    setModified(true);
    emit tapeContentChanged();
}

void TapeDocument::setInitialHeadPosition(int position)
{
    m_initialHeadPosition = position;
    m_tape->setHeadPosition(position);

    setModified(true);
    emit tapeContentChanged();
}

bool TapeDocument::step(bool isManualStep)
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        qWarning() << "No active machine available for step";
        return false;
    }

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Execute a step
    bool success = machine->step();

    // Make sure the machine is in PAUSED state after a manual step
    if (success && machine->getStatus() == ExecutionStatus::RUNNING && isManualStep) {
        machine->pause();
    }

    emit executionStateChanged();
    return success;
}

void TapeDocument::reset()
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        return;
    }

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Reset the machine
    machine->reset(true);

    emit executionStateChanged();
    emit tapeContentChanged();
}

void TapeDocument::run()
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        return;
    }

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Set the status to running
    machine->run();

    emit executionStateChanged();
}

void TapeDocument::pause()
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        return;
    }

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Pause the machine
    machine->pause();

    emit executionStateChanged();
}

void TapeDocument::stop()
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        return;
    }

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Stop the simulation (set status to READY)
    machine->reset();

    // Restore the initial head position but don't modify tape content
    m_tape->setHeadPosition(m_initialHeadPosition);

    emit executionStateChanged();
}

bool TapeDocument::canStepBackward() const
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        return false;
    }

    return machine->canStepBackward();
}

bool TapeDocument::stepBackward()
{
    TuringMachine* machine = getActiveMachine();
    if (!machine) {
        return false;
    }

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Step backward
    bool success = machine->stepBackward();

    emit executionStateChanged();
    return success;
}

bool TapeDocument::loadFromFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open tape file for reading:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in tape file";
        file.close();
        return false;
    }

    QJsonObject rootObj = doc.object();

    // Load initial content if present
    if (rootObj.contains("initialContent")) {
        m_initialContent = rootObj["initialContent"].toString().toStdString();
    } else {
        m_initialContent = "";
    }

    // Load initial head position if present
    if (rootObj.contains("initialHeadPosition")) {
        m_initialHeadPosition = rootObj["initialHeadPosition"].toInt();
    } else {
        m_initialHeadPosition = 0;
    }

    // Load tape cells if present
    if (rootObj.contains("cells") && rootObj["cells"].isArray()) {
        QJsonArray cellsArray = rootObj["cells"].toArray();
        std::map<int, std::string> cellsMap;

        for (const QJsonValue& cellValue : cellsArray) {
            QJsonObject cellObj = cellValue.toObject();
            int pos = cellObj["pos"].toInt();
            std::string val = cellObj["val"].toString().toStdString();
            cellsMap[pos] = val;
        }

        // Set the tape content
        m_tape->setContentFromMap(cellsMap);
    } else {
        // Otherwise just use initial content
        m_tape->setInitialContent(m_initialContent);
    }

    // Set head position
    m_tape->setHeadPosition(m_initialHeadPosition);

    // Update document properties
    setFilePath(filePath);
    setModified(false);

    emit tapeContentChanged();

    return true;
}

bool TapeDocument::saveToFile(const std::string& filePath)
{
    QJsonObject rootObj;

    // Save initial content and head position
    rootObj["initialContent"] = QString::fromStdString(m_initialContent);
    rootObj["initialHeadPosition"] = m_initialHeadPosition;

    // Save cells
    QJsonArray cellsArray;
    auto cellsMap = m_tape->getAllNonBlankCells();

    for (const auto& cell : cellsMap) {
        QJsonObject cellObj;
        cellObj["pos"] = cell.first;
        cellObj["val"] = QString::fromStdString(cell.second);
        cellsArray.append(cellObj);
    }

    rootObj["cells"] = cellsArray;

    // Write to file
    QJsonDocument doc(rootObj);
    QFile file(QString::fromStdString(filePath));

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open tape file for writing:" << file.errorString();
        return false;
    }

    file.write(doc.toJson());
    file.close();

    return true;
}

void TapeDocument::onActiveMachineUpdated(TuringMachine* machine)
{
    if (machine) {
        // Set this document's tape as the active tape in the machine
        machine->setTape(m_tape.get());

        // Reset the machine but keep the tape content
        // Only reset if not in the middle of execution
        if (machine->getStatus() == ExecutionStatus::READY) {
            machine->reset(false);
        }

        // Notify that the execution state may have changed due to a new machine
        emit executionStateChanged();

        qDebug() << "Machine updated in TapeDocument, machine has"
                 << machine->getAllStates().size() << "states and"
                 << machine->getAllTransitions().size() << "transitions";
    }
}

TuringMachine* TapeDocument::getActiveMachine() const
{
    return SessionManager::getInstance().getActiveMachine();
}