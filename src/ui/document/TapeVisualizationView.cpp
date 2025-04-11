#include "TapeVisualizationView.h"
#include "../../document/TapeDocument.h"
#include "../../project/Project.h"
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
    connect(m_simulationTimer, &QTimer::timeout, this, &TapeVisualizationView::onSimulationTimerTick);

    // Connect to document signals
    if (m_tapeDocument) {
        connect(m_tapeDocument, &TapeDocument::executionStateChanged,
                this, &TapeVisualizationView::onExecutionStateChanged);
        connect(m_tapeDocument, &TapeDocument::tapeContentChanged,
                this, &TapeVisualizationView::onTapeContentChanged);
    }
}

TapeVisualizationView::~TapeVisualizationView()
{
    delete m_simulationTimer;
}

void TapeVisualizationView::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Header label
    QString headerText = m_tapeDocument ?
                         tr("Tape: %1").arg(QString::fromStdString(m_tapeDocument->getName())) :
                         tr("Tape Visualization");

    QLabel* headerLabel = new QLabel(headerText, this);
    QFont headerFont = headerLabel->font();
    headerFont.setBold(true);
    headerFont.setPointSize(headerFont.pointSize() + 1);
    headerLabel->setFont(headerFont);
    mainLayout->addWidget(headerLabel);

    // Tape widget
    m_tapeWidget = new TapeWidget(this);
    if (m_tapeDocument) {
        m_tapeWidget->setTape(m_tapeDocument->getTape());
    }
    m_tapeWidget->setMinimumHeight(150);
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
    if (!m_tapeDocument) return;

    // Update header label
    QString headerText = tr("Tape: %1").arg(QString::fromStdString(m_tapeDocument->getName()));
    if (m_tapeDocument->getProject()) {
        headerText += tr(" (Project: %1)").arg(QString::fromStdString(m_tapeDocument->getProject()->getName()));
    }
    QLabel* headerLabel = qobject_cast<QLabel*>(layout()->itemAt(0)->widget());
    if (headerLabel) {
        headerLabel->setText(headerText);
    }

    // Update the tape widget with the document's tape
    m_tapeWidget->setTape(m_tapeDocument->getTape());
    m_tapeWidget->updateTapeDisplay();

    // Update the content edit with the tape's content
    m_contentEdit->setText(QString::fromStdString(m_tapeDocument->getTape()->getCurrentContent()));

    // Update head position spin
    m_headPositionSpin->setValue(m_tapeDocument->getTape()->getHeadPosition());

    // Update simulation controls
    updateSimulationControls();

    setStatusMessage(tr("Tape loaded from document"));
}

void TapeVisualizationView::setTapeContent()
{
    if (!m_tapeDocument) {
        qDebug() << "No tape document available";
        return;
    }

    QString content = m_contentEdit->text();
    int headPos = m_headPositionSpin->value();

    qDebug() << "Setting tape content to:" << content << "with head at position:" << headPos;

    m_tapeDocument->setInitialContent(content.toStdString());
    m_tapeDocument->setInitialHeadPosition(headPos);

    // Force tape widget update
    if (m_tapeWidget) {
        m_tapeWidget->updateTapeDisplay();
        qDebug() << "TapeWidget display updated";
    } else {
        qDebug() << "No tape widget available";
    }

    setStatusMessage(tr("Tape content set to: ") + content);

    // Update simulation controls
    updateSimulationControls();

    emit viewModified();
}

void TapeVisualizationView::resetTape()
{
    if (!m_tapeDocument) return;

    m_tapeDocument->reset();
    m_tapeWidget->updateTapeDisplay();

    setStatusMessage(tr("Tape reset to initial state"));

    // Update simulation controls
    updateSimulationControls();
}

void TapeVisualizationView::runSimulation()
{
    if (!m_tapeDocument) return;

    // Start the simulation
    m_tapeDocument->run();

    // Start the timer to execute steps
    m_simulationTimer->start(m_simulationSpeed);

    // Update UI
    m_runButton->setEnabled(false);
    m_pauseButton->setEnabled(true);
    m_stepForwardButton->setEnabled(false);
    m_stepBackwardButton->setEnabled(false);

    setStatusMessage(tr("Simulation running..."));
}

void TapeVisualizationView::pauseSimulation()
{
    if (!m_tapeDocument) return;

    // Stop the timer
    m_simulationTimer->stop();

    // Pause the simulation
    m_tapeDocument->pause();

    // Update UI
    updateSimulationControls();

    setStatusMessage(tr("Simulation paused"));
}

void TapeVisualizationView::stepForward()
{
    if (!m_tapeDocument) return;

    // Execute a step
    bool success = m_tapeDocument->step();

    if (success) {
        m_tapeWidget->onStepExecuted();

        // Ensure controls are updated AFTER step execution
        QTimer::singleShot(0, this, &TapeVisualizationView::updateSimulationControls);

        setStatusMessage(tr("Step executed"));

        // Make sure run button is enabled after a successful step
        m_runButton->setEnabled(true);
        m_stepForwardButton->setEnabled(true);
    } else {
        // Step failed, machine might have halted
        updateSimulationControls();

        if (m_tapeDocument->getProject() && m_tapeDocument->getProject()->getMachine()) {
            auto status = m_tapeDocument->getProject()->getMachine()->getStatus();

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
        } else {
            setStatusMessage(tr("Step execution failed"), true);
        }
    }
}

void TapeVisualizationView::stepBackward()
{
    if (!m_tapeDocument) return;

    // Execute a backward step
    if (m_tapeDocument->stepBackward()) {
        m_tapeWidget->onStepExecuted();

        // Update UI based on execution state
        updateSimulationControls();

        setStatusMessage(tr("Step undone"));
    } else {
        // Step back failed
        updateSimulationControls();
        setStatusMessage(tr("Cannot step backward further"), true);
    }
}

void TapeVisualizationView::onTapeContentChanged()
{
    // Update tape display
    m_tapeWidget->updateTapeDisplay();

    // Mark as modified
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

void TapeVisualizationView::onSimulationTimerTick()
{
    // Execute a step during automatic simulation
    stepForward();

    // If the machine has halted or errored, stop the timer
    if (m_tapeDocument && m_tapeDocument->getProject() && m_tapeDocument->getProject()->getMachine()) {
        auto status = m_tapeDocument->getProject()->getMachine()->getStatus();

        if (status != ExecutionStatus::RUNNING && status != ExecutionStatus::PAUSED) {
            m_simulationTimer->stop();
            updateSimulationControls();
        }
    }
}

void TapeVisualizationView::onExecutionStateChanged()
{
    // Update the UI based on the current execution state
    updateSimulationControls();
}

void TapeVisualizationView::updateSimulationControls()
{
    if (!m_tapeDocument || !m_tapeDocument->getProject() || !m_tapeDocument->getProject()->getMachine()) {
        m_runButton->setEnabled(false);
        m_pauseButton->setEnabled(false);
        m_stepForwardButton->setEnabled(false);
        m_stepBackwardButton->setEnabled(false);
        return;
    }

    auto machine = m_tapeDocument->getProject()->getMachine();
    auto status = machine->getStatus();

    qDebug() << "Machine status:" << static_cast<int>(status);

    // Always enable step forward for READY and PAUSED states
    m_stepForwardButton->setEnabled(status == ExecutionStatus::READY || status == ExecutionStatus::PAUSED);

    // Enable/disable Run button
    m_runButton->setEnabled(status == ExecutionStatus::READY || status == ExecutionStatus::PAUSED);

    // Enable/disable Pause button
    m_pauseButton->setEnabled(status == ExecutionStatus::RUNNING);

    // Enable/disable Step Backward button
    m_stepBackwardButton->setEnabled(m_tapeDocument->canStepBackward());
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