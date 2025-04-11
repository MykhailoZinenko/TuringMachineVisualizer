#include "TapeDocument.h"
#include "../project/Project.h"
#include "../model/TuringMachine.h"
#include "../model/Tape.h"
#include <QDebug>

TapeDocument::TapeDocument(Project* project, const std::string& id, const std::string& name)
    : Document(project, DocumentType::TAPE, name),
      m_initialHeadPosition(0)
{
    // Create a new tape
    m_tape = std::make_unique<Tape>();
}

TapeDocument::~TapeDocument()
{
}

void TapeDocument::setInitialContent(const std::string& content)
{
    m_initialContent = content;
    m_tape->setInitialContent(content);

    qDebug() << "Setting tape content to:" << &m_tape << " " << content;

    if (getProject()) {
        getProject()->setModified(true);
    }

    emit tapeContentChanged();
}

void TapeDocument::setInitialHeadPosition(int position)
{
    m_initialHeadPosition = position;
    m_tape->setHeadPosition(position);

    if (getProject()) {
        getProject()->setModified(true);
    }

    emit tapeContentChanged();
}

bool TapeDocument::step()
{
    if (!getProject() || !getProject()->getMachine()) {
        qWarning() << "No machine available for step";
        return false;
    }

    // Set the active tape in the machine
    TuringMachine* machine = getProject()->getMachine();
    Tape* tape = m_tape.get();

    // Create a temporary link to our tape
    machine->setTape(tape);

    // Execute a step
    bool success = machine->step();

    // Make sure the machine is in PAUSED state after a manual step
    if (success && machine->getStatus() == ExecutionStatus::RUNNING) {
        machine->pause();
    }

    emit executionStateChanged();
    return success;
}

void TapeDocument::reset()
{
    if (!getProject() || !getProject()->getMachine()) {
        return;
    }

    TuringMachine* machine = getProject()->getMachine();

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Reset the machine
    machine->reset();

    emit executionStateChanged();
}

void TapeDocument::run()
{
    if (!getProject() || !getProject()->getMachine()) {
        return;
    }

    TuringMachine* machine = getProject()->getMachine();

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Set the status to running
    machine->run();

    emit executionStateChanged();
}

void TapeDocument::pause()
{
    if (!getProject() || !getProject()->getMachine()) {
        return;
    }

    TuringMachine* machine = getProject()->getMachine();

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Pause the machine
    machine->pause();

    emit executionStateChanged();
}

bool TapeDocument::canStepBackward() const
{
    if (!getProject() || !getProject()->getMachine()) {
        return false;
    }

    return getProject()->getMachine()->canStepBackward();
}

bool TapeDocument::stepBackward()
{
    if (!getProject() || !getProject()->getMachine()) {
        return false;
    }

    TuringMachine* machine = getProject()->getMachine();

    // Set the active tape in the machine
    machine->setTape(m_tape.get());

    // Step backward
    bool success = machine->stepBackward();

    emit executionStateChanged();
    return success;
}