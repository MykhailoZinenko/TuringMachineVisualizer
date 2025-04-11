#include "TapeVisualizationView.h"
#include "../../document/TapeDocument.h"
#include "../../document/CodeDocument.h"
#include "../TapeWidget.h"
#include "../../model/TuringMachine.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QSlider>

TapeVisualizationView::TapeVisualizationView(TapeDocument* document, QWidget* parent)
    : DocumentView(document, parent),
      m_tapeDocument(document),
      m_simulationSpeed(500) // Default speed: 500ms
{
    setupUI();
    updateFromDocument();
    
    // Create simulation timer
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout, this, &TapeVisualizationView::stepForward);
}

TapeVisualizationView::~TapeVisualizationView()
{
    delete m_simulationTimer;
}

// Update setupUI method to better integrate TapeWidget
void TapeVisualizationView::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Header with code reference
    QLabel* headerLabel = new QLabel(tr("Tape: %1 (Machine: %2)")
        .arg(QString::fromStdString(m_tapeDocument->getName()))
        .arg(QString::fromStdString(m_tapeDocument->getCodeDocument()->getName())), this);
    QFont headerFont = headerLabel->font();
    headerFont.setBold(true);
    headerFont.setPointSize(headerFont.pointSize() + 1);
    headerLabel->setFont(headerFont);
    mainLayout->addWidget(headerLabel);

    // Tape widget
    m_tapeWidget = new TapeWidget(this);
    m_tapeWidget->setMinimumHeight(150);
    m_tapeWidget->setTape(m_tapeDocument->getTape());
    mainLayout->addWidget(m_tapeWidget, 1);

    // Tape content setup group
    QGroupBox* contentGroup = new QGroupBox(tr("Tape Setup"), this);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentGroup);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel(tr("Initial Content:"), this));
    m_contentEdit = new QLineEdit(this);
    inputLayout->addWidget(m_contentEdit, 1);

    inputLayout->addWidget(new QLabel(tr("Head Position:"), this));
    m_headPositionSpin = new QSpinBox(this);
    m_headPositionSpin->setMinimum(0);
    m_headPositionSpin->setMaximum(999);
    inputLayout->addWidget(m_headPositionSpin);

    contentLayout->addLayout(inputLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_setButton = new QPushButton(tr("Set Tape"), this);
    connect(m_setButton, &QPushButton::clicked, this, &TapeVisualizationView::setTapeContent);
    buttonLayout->addWidget(m_setButton);

    m_resetButton = new QPushButton(tr("Reset Tape"), this);
    connect(m_resetButton, &QPushButton::clicked, this, &TapeVisualizationView::resetTape);
    buttonLayout->addWidget(m_resetButton);

    // Add zoom controls
    QPushButton* zoomInButton = new QPushButton(tr("+"), this);
    connect(zoomInButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::zoomIn);
    buttonLayout->addWidget(zoomInButton);

    QPushButton* zoomOutButton = new QPushButton(tr("-"), this);
    connect(zoomOutButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::zoomOut);
    buttonLayout->addWidget(zoomOutButton);

    QPushButton* resetZoomButton = new QPushButton(tr("Reset Zoom"), this);
    connect(resetZoomButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::resetZoom);
    buttonLayout->addWidget(resetZoomButton);

    contentLayout->addLayout(buttonLayout);
    mainLayout->addWidget(contentGroup);

    // Simulation controls group
    QGroupBox* simulationGroup = new QGroupBox(tr("Simulation"), this);
    QVBoxLayout* simulationLayout = new QVBoxLayout(simulationGroup);

    QHBoxLayout* controlsLayout = new QHBoxLayout();
    m_runButton = new QPushButton(tr("Run"), this);
    connect(m_runButton, &QPushButton::clicked, this, &TapeVisualizationView::runSimulation);
    controlsLayout->addWidget(m_runButton);

    m_pauseButton = new QPushButton(tr("Pause"), this);
    connect(m_pauseButton, &QPushButton::clicked, this, &TapeVisualizationView::pauseSimulation);
    m_pauseButton->setEnabled(false);
    controlsLayout->addWidget(m_pauseButton);

    m_stepForwardButton = new QPushButton(tr("Step >"), this);
    connect(m_stepForwardButton, &QPushButton::clicked, this, &TapeVisualizationView::stepForward);
    controlsLayout->addWidget(m_stepForwardButton);

    m_stepBackwardButton = new QPushButton(tr("< Step"), this);
    connect(m_stepBackwardButton, &QPushButton::clicked, this, &TapeVisualizationView::stepBackward);
    m_stepBackwardButton->setEnabled(false);
    controlsLayout->addWidget(m_stepBackwardButton);

    simulationLayout->addLayout(controlsLayout);

    // Speed slider
    QHBoxLayout* speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel(tr("Speed:"), this));

    QSlider* speedSlider = new QSlider(Qt::Horizontal, this);
    speedSlider->setRange(50, 1000);
    speedSlider->setValue(m_simulationSpeed);
    speedSlider->setInvertedAppearance(true);  // Faster to the right
    connect(speedSlider, &QSlider::valueChanged, this, &TapeVisualizationView::onSimulationSpeed);
    speedLayout->addWidget(speedSlider, 1);

    QLabel* speedValueLabel = new QLabel(QString::number(m_simulationSpeed) + " ms", this);
    connect(speedSlider, &QSlider::valueChanged,
            [speedValueLabel](int value) { speedValueLabel->setText(QString::number(value) + " ms"); });
    speedLayout->addWidget(speedValueLabel);

    simulationLayout->addLayout(speedLayout);

    mainLayout->addWidget(simulationGroup);

    // Status label
    m_statusLabel = new QLabel(tr("Ready"), this);
    mainLayout->addWidget(m_statusLabel);

    // Connect tape signals
    connect(m_tapeWidget, &TapeWidget::tapeModified, this, &TapeVisualizationView::onTapeContentChanged);
}

void TapeVisualizationView::updateFromDocument()
{
    // Update the tape widget with the document's tape
    m_tapeWidget->setTape(m_tapeDocument->getTape());
    m_tapeWidget->updateTapeDisplay();
    
    // Update the content edit with the tape's initial content
    m_contentEdit->setText(QString::fromStdString(m_tapeDocument->getTape()->getCurrentContent()));
    
    // Update head position spin
    m_headPositionSpin->setValue(m_tapeDocument->getTape()->getHeadPosition());
    
    setStatusMessage(tr("Tape loaded from document"));
}

void TapeVisualizationView::setTapeContent()
{
    QString content = m_contentEdit->text();
    int headPos = m_headPositionSpin->value();
    
    Tape* tape = m_tapeDocument->getTape();
    tape->reset();
    tape->setInitialContent(content.toStdString());
    tape->setHeadPosition(headPos);
    
    m_tapeWidget->updateTapeDisplay();
    
    m_tapeDocument->setInitialContent(content.toStdString());
    m_tapeDocument->setInitialHeadPosition(headPos);
    
    setStatusMessage(tr("Tape content set"));
    
    // Update simulation controls
    updateSimulationControls();
    
    emit viewModified();
}

void TapeVisualizationView::resetTape()
{
    Tape* tape = m_tapeDocument->getTape();
    tape->reset();
    
    // If we have initial content, apply it
    tape->setInitialContent(m_tapeDocument->getTape()->getCurrentContent());
    tape->setHeadPosition(m_tapeDocument->getTape()->getHeadPosition());
    
    m_tapeWidget->updateTapeDisplay();
    
    setStatusMessage(tr("Tape reset to initial state"));
    
    // Update simulation controls
    updateSimulationControls();
}

void TapeVisualizationView::runSimulation()
{
    // Access the Turing machine from the code document
    TuringMachine* machine = m_tapeDocument->getCodeDocument()->getMachine();
    
    if (!machine) {
        setStatusMessage(tr("No machine available"), true);
        return;
    }
    
    if (machine->getStatus() == ExecutionStatus::HALTED_ACCEPT ||
        machine->getStatus() == ExecutionStatus::HALTED_REJECT ||
        machine->getStatus() == ExecutionStatus::ERROR) {
        setStatusMessage(tr("Machine halted, reset to run again"), true);
        return;
    }
    
    m_simulationTimer->start(m_simulationSpeed);
    
    m_runButton->setEnabled(false);
    m_pauseButton->setEnabled(true);
    m_stepForwardButton->setEnabled(false);
    m_stepBackwardButton->setEnabled(false);
    
    setStatusMessage(tr("Simulation running..."));
}

void TapeVisualizationView::pauseSimulation()
{
    m_simulationTimer->stop();
    
    m_runButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stepForwardButton->setEnabled(true);
    
    // Check if we can step backward
    TuringMachine* machine = m_tapeDocument->getCodeDocument()->getMachine();
    if (machine && machine->canStepBackward()) {
        m_stepBackwardButton->setEnabled(true);
    }
    
    setStatusMessage(tr("Simulation paused"));
}

void TapeVisualizationView::stepForward()
{
    TuringMachine* machine = m_tapeDocument->getCodeDocument()->getMachine();
    
    if (!machine) {
        setStatusMessage(tr("No machine available"), true);
        return;
    }

    machine->setActiveTape(m_tapeDocument->getTape());
    
    if (machine->step()) {
        m_tapeWidget->onStepExecuted();
        
        // Update backward button based on history
        m_stepBackwardButton->setEnabled(machine->canStepBackward());
        
        setStatusMessage(tr("Step executed - State: %1")
            .arg(QString::fromStdString(machine->getCurrentState())));
    } else {
        // Machine halted
        m_simulationTimer->stop();
        
        ExecutionStatus status = machine->getStatus();
        switch (status) {
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
                setStatusMessage(tr("Machine halted"), true);
                break;
        }
        
        m_runButton->setEnabled(false);
        m_pauseButton->setEnabled(false);
        m_stepForwardButton->setEnabled(false);
        m_stepBackwardButton->setEnabled(machine->canStepBackward());
    }
}

void TapeVisualizationView::stepBackward()
{
    TuringMachine* machine = m_tapeDocument->getCodeDocument()->getMachine();
    
    if (!machine) {
        setStatusMessage(tr("No machine available"), true);
        return;
    }
    
    if (machine->stepBackward()) {
        m_tapeWidget->onStepExecuted();
        
        // Update backward button
        m_stepBackwardButton->setEnabled(machine->canStepBackward());
        
        setStatusMessage(tr("Step undone - State: %1")
            .arg(QString::fromStdString(machine->getCurrentState())));
    } else {
        setStatusMessage(tr("Cannot step backward further"), true);
        m_stepBackwardButton->setEnabled(false);
    }
}

void TapeVisualizationView::onTapeContentChanged()
{
    emit viewModified();
}

void TapeVisualizationView::onSimulationSpeed(int value)
{
    m_simulationSpeed = value;
    
    if (m_simulationTimer->isActive()) {
        m_simulationTimer->setInterval(m_simulationSpeed);
    }
    
    setStatusMessage(tr("Simulation speed set to %1 ms").arg(m_simulationSpeed));
}

void TapeVisualizationView::updateSimulationControls()
{
    // Reset simulation controls to default state
    m_runButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stepForwardButton->setEnabled(true);
    m_stepBackwardButton->setEnabled(false);
    
    // Check machine state
    TuringMachine* machine = m_tapeDocument->getCodeDocument()->getMachine();
    if (machine) {
        ExecutionStatus status = machine->getStatus();
        
        if (status == ExecutionStatus::HALTED_ACCEPT ||
            status == ExecutionStatus::HALTED_REJECT ||
            status == ExecutionStatus::ERROR) {
            m_runButton->setEnabled(false);
            m_stepForwardButton->setEnabled(false);
        }
        
        if (machine->canStepBackward()) {
            m_stepBackwardButton->setEnabled(true);
        }
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