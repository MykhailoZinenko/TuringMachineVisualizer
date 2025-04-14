#include "CodeDocument.h"
#include "../model/TuringMachine.h"
#include "../parser/CodeParser.h"
#include "../core/FileWatcher.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "src/core/SessionManager.h"

CodeDocument::CodeDocument(const std::string& name, const std::string& filePath)
    : Document(DocumentType::CODE, name, filePath)
{
    // Create a new empty Turing machine
    m_machine = std::make_unique<TuringMachine>(name);

    // Set up file watching if we have a path
    if (!filePath.empty()) {
        setupFileWatching();
    }

    // Connect to our own filePathChanged signal to handle file watching
    connect(this, &Document::filePathChanged, [this](const std::string& path) {
        if (!path.empty()) {
            setupFileWatching();
        } else {
            removeFileWatching();
        }
    });
}

CodeDocument::~CodeDocument()
{
    removeFileWatching();
}

std::string CodeDocument::getCode() const
{
    return m_code;
}

void CodeDocument::setCode(const std::string& code, bool shouldUpdateMachine)
{
    if (m_code != code) {
        m_code = code;
        setModified(true);
        emit contentChanged();

        if (shouldUpdateMachine) {
            updateMachine();
        }
    }
}

TuringMachine* CodeDocument::getMachine() const
{
    return m_machine.get();
}

bool CodeDocument::loadFromFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << file.errorString();
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Update document properties
    setFilePath(filePath);
    setCode(content.toStdString(), true);
    setModified(false);

    return true;
}

bool CodeDocument::saveToFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << QString::fromStdString(m_code);
    file.close();

    return true;
}

void CodeDocument::onFileChanged(const QString& filePath)
{
    if (filePath.toStdString() != getFilePath()) {
        return; // Not our file
    }

    qDebug() << "File changed on disk:" << filePath;

    // Reload the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open changed file for reading:" << file.errorString();
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Update the code without marking as modified (it was saved externally)
    m_code = content.toStdString();
    setModified(false);

    // Update the machine
    if (updateMachine()) {
        qDebug() << "Machine updated from external file change";
    } else {
        qWarning() << "Failed to update machine from external file change";
    }

    // Notify that content changed
    emit contentChanged();
}

bool CodeDocument::updateMachine()
{
    // Use the CodeParser to update the machine
    CodeParser parser;
    bool success = parser.parseAndUpdateMachine(m_machine.get(), m_code);

    if (success) {
        // Store the original code in the machine
        m_machine->setOriginalCode(m_code);

        // Notify that the machine was updated
        emit machineUpdated(m_machine.get());

        // Update the SessionManager to notify all listeners
        // This ensures that when code is saved, the active tape immediately gets the changes
        auto& sessionManager = SessionManager::getInstance();
        if (sessionManager.getActiveCodeDocument() == this) {
            sessionManager.activeMachineUpdated(m_machine.get());
        }

        qDebug() << "Machine updated in CodeDocument, machine has"
                 << m_machine->getAllStates().size() << "states and"
                 << m_machine->getAllTransitions().size() << "transitions";
    }

    return success;
}

void CodeDocument::setupFileWatching()
{
    if (!getFilePath().empty()) {
        // Start watching the file
        FileWatcher::getInstance().watchFile(QString::fromStdString(getFilePath()));

        // Connect to file changed signal
        connect(&FileWatcher::getInstance(), &FileWatcher::fileChanged,
                this, &CodeDocument::onFileChanged);
    }
}

void CodeDocument::removeFileWatching()
{
    if (!getFilePath().empty()) {
        // Stop watching the file
        FileWatcher::getInstance().unwatchFile(QString::fromStdString(getFilePath()));

        // Disconnect from file changed signal
        disconnect(&FileWatcher::getInstance(), &FileWatcher::fileChanged,
                  this, &CodeDocument::onFileChanged);
    }
}