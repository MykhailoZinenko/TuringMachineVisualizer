#include "TapeVisualizationView.h"
#include "../document/TapeDocument.h"
#include "../model/Tape.h"
#include "../model/TuringMachine.h"
#include "../core/SessionManager.h"
#include "TapeWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QDebug>

TapeVisualizationView::TapeVisualizationView(TapeDocument* document, QWidget* parent)
    : QWidget(parent), m_document(document), m_simulationSpeed(500)
{
    setupUI();

    // Create simulation timer
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout,
            this, &TapeVisualizationView::onSimulationTimerTick);

    if (m_document) {
        updateFromDocument();

        // Connect document signals
        connect(m_document, &TapeDocument::tapeContentChanged,
                this, &TapeVisualizationView::onTapeContentChanged);
        connect(m_document, &TapeDocument::executionStateChanged,
                this, &TapeVisualizationView::onExecutionStateChanged);
        connect(m_document, &Document::documentSaved,
                this, &TapeVisualizationView::onDocumentSaved);
    }

    // Connect to SessionManager for machine updates
    connect(&SessionManager::getInstance(), &SessionManager::activeMachineUpdated,
            this, &TapeVisualizationView::onActiveMachineUpdated);
}

TapeVisualizationView::~TapeVisualizationView()
{
    // Stop the timer if running
    if (m_simulationTimer->isActive()) {
        m_simulationTimer->stop();
    }
}

void TapeVisualizationView::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Tape visualization
    m_tapeWidget = new TapeWidget(this);
    m_tapeWidget->setMinimumHeight(100);
    mainLayout->addWidget(m_tapeWidget, 1);

    // Connect tape widget signals
    connect(m_tapeWidget, &TapeWidget::tapeModified,
            this, &TapeVisualizationView::onTapeModified);
    connect(m_tapeWidget, &TapeWidget::cellValueChanged,
            this, &TapeVisualizationView::onCellValueChanged);
    connect(m_tapeWidget, &TapeWidget::headPositionChanged,
            this, &TapeVisualizationView::onHeadPositionChanged);

    // Create controls
    QGroupBox* controlsGroup = new QGroupBox(tr("Tape Controls"), this);
    QVBoxLayout* controlsLayout = new QVBoxLayout(controlsGroup);

    // Tape content controls
    QGridLayout* tapeLayout = new QGridLayout();

    m_contentEdit = new QLineEdit(this);
    tapeLayout->addWidget(new QLabel(tr("Content:")), 0, 0);
    tapeLayout->addWidget(m_contentEdit, 0, 1);

    m_headPositionSpin = new QSpinBox(this);
    m_headPositionSpin->setRange(0, 999999);
    tapeLayout->addWidget(new QLabel(tr("Head Position:")), 1, 0);
    tapeLayout->addWidget(m_headPositionSpin, 1, 1);

    m_setButton = new QPushButton(tr("Set"), this);
    m_resetButton = new QPushButton(tr("Reset"), this);
    QHBoxLayout* tapeButtonLayout = new QHBoxLayout();
    tapeButtonLayout->addWidget(m_setButton);
    tapeButtonLayout->addWidget(m_resetButton);
    tapeLayout->addLayout(tapeButtonLayout, 2, 0, 1, 2);

    controlsLayout->addLayout(tapeLayout);

    // Simulation controls
    QGroupBox* simulationGroup = new QGroupBox(tr("Simulation"), this);
    QVBoxLayout* simulationLayout = new QVBoxLayout(simulationGroup);

    QHBoxLayout* runLayout = new QHBoxLayout();
    m_runButton = new QPushButton(tr("Run"), this);
    m_pauseButton = new QPushButton(tr("Pause"), this);
    m_stopButton = new QPushButton(tr("Stop"), this);
    m_stepForwardButton = new QPushButton(tr("Step >"), this);
    m_stepBackwardButton = new QPushButton(tr("< Step"), this);

    runLayout->addWidget(m_runButton);
    runLayout->addWidget(m_pauseButton);
    runLayout->addWidget(m_stopButton);
    runLayout->addWidget(m_stepBackwardButton);
    runLayout->addWidget(m_stepForwardButton);

    simulationLayout->addLayout(runLayout);

    // Speed slider
    QHBoxLayout* speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel(tr("Speed:")));

    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(50, 1000);
    m_speedSlider->setValue(m_simulationSpeed);
    m_speedSlider->setInvertedAppearance(true); // Faster to the right
    speedLayout->addWidget(m_speedSlider, 1);

    m_speedLabel = new QLabel(tr("%1 ms").arg(m_simulationSpeed), this);
    speedLayout->addWidget(m_speedLabel);

    simulationLayout->addLayout(speedLayout);

    controlsLayout->addWidget(simulationGroup);

    // Status label
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_statusLabel->setMinimumHeight(24);

    // Add to main layout
    mainLayout->addWidget(controlsGroup);
    mainLayout->addWidget(m_statusLabel);

    // Connect signals
    connect(m_setButton, &QPushButton::clicked, this, &TapeVisualizationView::setTapeContent);
    connect(m_resetButton, &QPushButton::clicked, this, &TapeVisualizationView::resetTape);
    connect(m_runButton, &QPushButton::clicked, this, &TapeVisualizationView::runSimulation);
    connect(m_pauseButton, &QPushButton::clicked, this, &TapeVisualizationView::pauseSimulation);
    connect(m_stopButton, &QPushButton::clicked, this, &TapeVisualizationView::stopSimulation);
    connect(m_stepForwardButton, &QPushButton::clicked, this, &TapeVisualizationView::stepForward);
    connect(m_stepBackwardButton, &QPushButton::clicked, this, &TapeVisualizationView::stepBackward);
    connect(m_speedSlider, &QSlider::valueChanged, this, &TapeVisualizationView::onSimulationSpeedChanged);

    // Initial state
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
    m_stepBackwardButton->setEnabled(false);
}

void TapeVisualizationView::updateFromDocument()
{
    if (!m_document) {
        return;
    }

    // Update tape widget
    m_tapeWidget->setTape(m_document->getTape());
    m_tapeWidget->updateTapeDisplay();

    // Update content edit
    if (m_document->getTape()) {
        m_contentEdit->setText(QString::fromStdString(m_document->getTape()->getCurrentContent()));
        m_headPositionSpin->setValue(m_document->getTape()->getHeadPosition());
    }

    // Update simulation controls
    updateSimulationControls();

    // Update status
    setStatusMessage(tr("Document loaded"));
}

void TapeVisualizationView::onTapeContentChanged()
{
    if (!m_document) {
        return;
    }

    // Update tape widget
    m_tapeWidget->updateTapeDisplay();

    // Update content edit
    if (m_document->getTape()) {
        m_contentEdit->setText(QString::fromStdString(m_document->getTape()->getCurrentContent()));
        m_headPositionSpin->setValue(m_document->getTape()->getHeadPosition());
    }

    // Notify that the view content has changed
    emit viewModified();
}

void TapeVisualizationView::onExecutionStateChanged()
{
    updateSimulationControls();
}

void TapeVisualizationView::onDocumentSaved(const std::string& filePath)
{
    Q_UNUSED(filePath);

    // Update status
    setStatusMessage(tr("Document saved"));

    // Notify that the view has been saved
    emit viewSaved();
}

void TapeVisualizationView::onTapeModified()
{
    if (!m_document || !m_document->getTape()) {
        return;
    }

    // Update content edit to match tape
    m_contentEdit->setText(QString::fromStdString(m_document->getTape()->getCurrentContent()));
    m_headPositionSpin->setValue(m_document->getTape()->getHeadPosition());

    // Mark document as modified
    emit viewModified();
}

void TapeVisualizationView::onCellValueChanged(int position, const std::string& newValue)
{
    Q_UNUSED(position);
    Q_UNUSED(newValue);

    // Just update UI
    onTapeModified();
}

void TapeVisualizationView::onHeadPositionChanged(int newPosition)
{
    Q_UNUSED(newPosition);

    // Just update UI
    onTapeModified();
}

void TapeVisualizationView::setTapeContent()
{
    if (!m_document) {
        return;
    }

    // Get content from UI
    QString content = m_contentEdit->text();
    int headPosition = m_headPositionSpin->value();

    // Update document
    m_document->setInitialContent(content.toStdString());
    m_document->setInitialHeadPosition(headPosition);

    // Update tape widget
    m_tapeWidget->updateTapeDisplay();

    // Update status
    setStatusMessage(tr("Tape content set"));

    // Notify that the view content has changed
    emit viewModified();
}

void TapeVisualizationView::resetTape()
{
    if (!m_document) {
        return;
    }

    // Reset the document
    m_document->reset();

    // Update tape widget
    m_tapeWidget->updateTapeDisplay();

    // Update content edit
    if (m_document->getTape()) {
        m_contentEdit->setText(QString::fromStdString(m_document->getTape()->getCurrentContent()));
        m_headPositionSpin->setValue(m_document->getTape()->getHeadPosition());
    }

    // Update status
    setStatusMessage(tr("Tape reset"));
}

void TapeVisualizationView::runSimulation()
{
    if (!m_document) {
        return;
    }

    // Start the simulation
    m_document->run();

    // Start the timer
    m_simulationTimer->start(m_simulationSpeed);

    // Update controls
    m_runButton->setEnabled(false);
    m_pauseButton->setEnabled(true);
    m_stopButton->setEnabled(true);
    m_stepForwardButton->setEnabled(false);
    m_stepBackwardButton->setEnabled(false);

    // Update status
    setStatusMessage(tr("Simulation running..."));
}

void TapeVisualizationView::pauseSimulation()
{
    if (!m_document) {
        return;
    }

    // Stop the timer
    m_simulationTimer->stop();

    // Pause the simulation
    m_document->pause();

    // Update controls
    updateSimulationControls();

    // Update status
    setStatusMessage(tr("Simulation paused"));
}

void TapeVisualizationView::stopSimulation()
{
    if (!m_document) {
        return;
    }

    // Stop the timer
    m_simulationTimer->stop();

    // Stop the simulation
    m_document->stop();

    // Update controls
    updateSimulationControls();

    // Update status
    setStatusMessage(tr("Simulation stopped"));
}

void TapeVisualizationView::stepForward()
{
    if (!m_document) {
        return;
    }

    // Execute a step
    bool success = m_document->step(true);

    // Update tape widget
    m_tapeWidget->onStepExecuted();

    // Update controls
    updateSimulationControls();

    if (success) {
        // Update status
        setStatusMessage(tr("Step executed"));
    } else {
        // Get the machine status
        TuringMachine* machine = SessionManager::getInstance().getActiveMachine();
        if (machine) {
            switch (machine->getStatus()) {
                case ExecutionStatus::HALTED_ACCEPT:
                    setStatusMessage(tr("Machine halted: Accept state reached"));
                    break;
                case ExecutionStatus::HALTED_REJECT:
                    setStatusMessage(tr("Machine halted: Reject state reached"));
                    break;
                case ExecutionStatus::ERROR:
                    setStatusMessage(tr("Machine halted: No valid transition"), true);
                    break;
                default:
                    setStatusMessage(tr("Step failed"), true);
                    break;
            }
        } else {
            setStatusMessage(tr("Step failed: No active machine"), true);
        }
    }
}

void TapeVisualizationView::stepBackward()
{
    if (!m_document) {
        return;
    }

    // Execute a step backward
    bool success = m_document->stepBackward();

    // Update tape widget
    m_tapeWidget->onStepExecuted();

    // Update controls
    updateSimulationControls();

    if (success) {
        // Update status
        setStatusMessage(tr("Step undone"));
    } else {
        // Update status
        setStatusMessage(tr("Cannot step backward further"), true);
    }
}

void TapeVisualizationView::onSimulationSpeedChanged(int value)
{
    m_simulationSpeed = value;
    m_speedLabel->setText(tr("%1 ms").arg(value));

    if (m_simulationTimer->isActive()) {
        m_simulationTimer->setInterval(m_simulationSpeed);
    }
}

void TapeVisualizationView::onSimulationTimerTick()
{
    if (!m_document) {
        return;
    }

    // Execute a step
    bool success = m_document->step(false);

    // Update tape widget
    m_tapeWidget->onStepExecuted();

    if (!success) {
        // Stop the timer
        m_simulationTimer->stop();

        // Update controls
        updateSimulationControls();

        // Get the machine status
        TuringMachine* machine = SessionManager::getInstance().getActiveMachine();
        if (machine) {
            switch (machine->getStatus()) {
                case ExecutionStatus::HALTED_ACCEPT:
                    setStatusMessage(tr("Machine halted: Accept state reached"));
                    break;
                case ExecutionStatus::HALTED_REJECT:
                    setStatusMessage(tr("Machine halted: Reject state reached"));
                    break;
                case ExecutionStatus::ERROR:
                    setStatusMessage(tr("Machine halted: No valid transition"), true);
                    break;
                default:
                    setStatusMessage(tr("Simulation halted"), true);
                    break;
            }
        } else {
            setStatusMessage(tr("Simulation halted: No active machine"), true);
        }
    }
}

void TapeVisualizationView::onActiveMachineUpdated(TuringMachine* machine)
{
    Q_UNUSED(machine);

    // Update simulation controls
    updateSimulationControls();

    // Update status if we have an active machine
    if (SessionManager::getInstance().getActiveMachine()) {
        int states = SessionManager::getInstance().getActiveMachine()->getAllStates().size();
        int transitions = SessionManager::getInstance().getActiveMachine()->getAllTransitions().size();

        setStatusMessage(tr("Machine active: %1 states, %2 transitions")
                        .arg(states)
                        .arg(transitions));
    }
}

void TapeVisualizationView::updateSimulationControls()
{
    // Get the active machine
    TuringMachine* machine = SessionManager::getInstance().getActiveMachine();

    if (!machine) {
        // No machine available
        m_runButton->setEnabled(false);
        m_pauseButton->setEnabled(false);
        m_stopButton->setEnabled(false);
        m_stepForwardButton->setEnabled(false);
        m_stepBackwardButton->setEnabled(false);
        return;
    }

    // Enable/disable controls based on machine status
    ExecutionStatus status = machine->getStatus();

    switch (status) {
        case ExecutionStatus::READY:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(false);
            m_stopButton->setEnabled(false);
            m_stepForwardButton->setEnabled(true);
            m_stepBackwardButton->setEnabled(m_document && m_document->canStepBackward());
            break;

        case ExecutionStatus::RUNNING:
            m_runButton->setEnabled(false);
            m_pauseButton->setEnabled(true);
            m_stopButton->setEnabled(true);
            m_stepForwardButton->setEnabled(false);
            m_stepBackwardButton->setEnabled(false);
            break;

        case ExecutionStatus::PAUSED:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(false);
            m_stopButton->setEnabled(true);
            m_stepForwardButton->setEnabled(true);
            m_stepBackwardButton->setEnabled(m_document && m_document->canStepBackward());
            break;

        case ExecutionStatus::HALTED_ACCEPT:
        case ExecutionStatus::HALTED_REJECT:
        case ExecutionStatus::ERROR:
            m_runButton->setEnabled(false);
            m_pauseButton->setEnabled(false);
            m_stopButton->setEnabled(true);
            m_stepForwardButton->setEnabled(false);
            m_stepBackwardButton->setEnabled(m_document && m_document->canStepBackward());
            break;
    }
}

void TapeVisualizationView::setStatusMessage(const QString& message, bool isError)
{
    m_statusLabel->setText(message);
    
    if (isError) {
        m_statusLabel->setStyleSheet("color: red;");
    } else {
        m_statusLabel->setStyleSheet("");
    }
}