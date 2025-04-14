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
#include <QShortcut>
#include <QSplitter>
#include <QToolBar>

#include "StyleKit.h"


class QToolBar;

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
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create IDE-style toolbar
    QToolBar* tapeToolbar = new QToolBar(this);
    tapeToolbar->setIconSize(QSize(16, 16));
    tapeToolbar->setStyleSheet(StyleKit::getToolbarStyle());

    // Create toolbar actions
    QAction* resetAction = new QAction(QIcon::fromTheme("edit-clear", QIcon(":/icons/reset.png")), tr("Reset"), this);
    QAction* runAction = new QAction(QIcon::fromTheme("media-playback-start", QIcon(":/icons/run.png")), tr("Run"), this);
    QAction* pauseAction = new QAction(QIcon::fromTheme("media-playback-pause", QIcon(":/icons/pause.png")), tr("Pause"), this);
    QAction* stopAction = new QAction(QIcon::fromTheme("media-playback-stop", QIcon(":/icons/stop.png")), tr("Stop"), this);
    QAction* stepBackAction = new QAction(QIcon::fromTheme("go-previous", QIcon(":/icons/step-back.png")), tr("Step Back"), this);
    QAction* stepForwardAction = new QAction(QIcon::fromTheme("go-next", QIcon(":/icons/step-forward.png")), tr("Step Forward"), this);

    // Add tooltips with keyboard shortcuts
    resetAction->setToolTip(tr("Reset tape (Ctrl+R)"));
    runAction->setToolTip(tr("Run simulation (F5)"));
    pauseAction->setToolTip(tr("Pause simulation (F6)"));
    stopAction->setToolTip(tr("Stop simulation (F7)"));
    stepBackAction->setToolTip(tr("Step backward (Ctrl+Left)"));
    stepForwardAction->setToolTip(tr("Step forward (Ctrl+Right)"));

    // Add actions to toolbar
    tapeToolbar->addAction(resetAction);
    tapeToolbar->addSeparator();
    tapeToolbar->addAction(runAction);
    tapeToolbar->addAction(pauseAction);
    tapeToolbar->addAction(stopAction);
    tapeToolbar->addSeparator();
    tapeToolbar->addAction(stepBackAction);
    tapeToolbar->addAction(stepForwardAction);

    // Tape visualization with IDE styling
    m_tapeWidget = new TapeWidget(this);
    m_tapeWidget->setMinimumHeight(150);
    m_tapeWidget->setStyleSheet(StyleKit::getTapeWidgetStyle());

    // Connect tape widget signals
    connect(m_tapeWidget, &TapeWidget::tapeModified,
            this, &TapeVisualizationView::onTapeModified);
    connect(m_tapeWidget, &TapeWidget::cellValueChanged,
            this, &TapeVisualizationView::onCellValueChanged);
    connect(m_tapeWidget, &TapeWidget::headPositionChanged,
            this, &TapeVisualizationView::onHeadPositionChanged);

    // Create tape configuration panel
    QWidget* configPanel = new QWidget(this);
    QVBoxLayout* configLayout = new QVBoxLayout(configPanel);
    configLayout->setContentsMargins(12, 12, 12, 12);
    configLayout->setSpacing(8);

    // Tape content controls with IDE styling
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(8);

    QLabel* contentLabel = new QLabel(tr("Content:"), this);
    m_contentEdit = new QLineEdit(this);
    m_contentEdit->setStyleSheet(StyleKit::getInputStyle());
    m_contentEdit->setPlaceholderText(tr("Enter initial tape content"));

    contentLayout->addWidget(contentLabel);
    contentLayout->addWidget(m_contentEdit);

    QHBoxLayout* positionLayout = new QHBoxLayout();
    positionLayout->setSpacing(8);

    QLabel* positionLabel = new QLabel(tr("Head Position:"), this);
    m_headPositionSpin = new QSpinBox(this);
    m_headPositionSpin->setRange(0, 999999);
    m_headPositionSpin->setStyleSheet(StyleKit::getInputStyle());

    positionLayout->addWidget(positionLabel);
    positionLayout->addWidget(m_headPositionSpin);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_setButton = new QPushButton(tr("Set"), this);
    m_setButton->setStyleSheet(StyleKit::getButtonStyle(true));

    buttonLayout->addWidget(m_setButton);
    buttonLayout->addStretch();

    configLayout->addLayout(contentLayout);
    configLayout->addLayout(positionLayout);
    configLayout->addLayout(buttonLayout);
    configLayout->addStretch();

    // Speed control
    QHBoxLayout* speedLayout = new QHBoxLayout();
    speedLayout->setSpacing(8);

    m_speedLabel = new QLabel(tr("Speed: %1 ms").arg(m_simulationSpeed), this);
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(50, 1000);
    m_speedSlider->setValue(m_simulationSpeed);
    m_speedSlider->setInvertedAppearance(true); // Faster to the right
    m_speedSlider->setStyleSheet(StyleKit::getSliderStyle());

    speedLayout->addWidget(m_speedLabel);
    speedLayout->addWidget(m_speedSlider, 1);

    configLayout->addLayout(speedLayout);

    // Status label with IDE styling
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setFrameStyle(QFrame::NoFrame);
    m_statusLabel->setMinimumHeight(24);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #F5F5F5;"  // Very light gray
        "  color: #333333;"             // Dark text
        "  border-top: 1px solid #BDBDBD;"  // Light gray top border
        "  padding: 4px 8px;"
        "}"
    );

    // Create a splitter for tape and config
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(m_tapeWidget);
    splitter->addWidget(configPanel);
    splitter->setStretchFactor(0, 2);  // Give tape more space
    splitter->setStretchFactor(1, 1);

    // Add everything to main layout
    mainLayout->addWidget(tapeToolbar);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(m_statusLabel);

    // Store action buttons for use outside the layout
    m_runButton = new QPushButton(this);
    m_pauseButton = new QPushButton(this);
    m_stopButton = new QPushButton(this);
    m_stepForwardButton = new QPushButton(this);
    m_stepBackwardButton = new QPushButton(this);
    m_resetButton = new QPushButton(this);

    // We're using actions instead of these buttons directly
    m_runButton->hide();
    m_pauseButton->hide();
    m_stopButton->hide();
    m_stepForwardButton->hide();
    m_stepBackwardButton->hide();
    m_resetButton->hide();

    // Connect toolbar actions
    connect(resetAction, &QAction::triggered, this, &TapeVisualizationView::resetTape);
    connect(runAction, &QAction::triggered, this, &TapeVisualizationView::runSimulation);
    connect(pauseAction, &QAction::triggered, this, &TapeVisualizationView::pauseSimulation);
    connect(stopAction, &QAction::triggered, this, &TapeVisualizationView::stopSimulation);
    connect(stepBackAction, &QAction::triggered, this, &TapeVisualizationView::stepBackward);
    connect(stepForwardAction, &QAction::triggered, this, &TapeVisualizationView::stepForward);

    // Connect other controls
    connect(m_setButton, &QPushButton::clicked, this, &TapeVisualizationView::setTapeContent);
    connect(m_speedSlider, &QSlider::valueChanged, this, &TapeVisualizationView::onSimulationSpeedChanged);

    // Set up keyboard shortcuts
    new QShortcut(QKeySequence("Ctrl+R"), this, SLOT(resetTape()));
    new QShortcut(QKeySequence("F5"), this, SLOT(runSimulation()));
    new QShortcut(QKeySequence("F6"), this, SLOT(pauseSimulation()));
    new QShortcut(QKeySequence("F7"), this, SLOT(stopSimulation()));
    new QShortcut(QKeySequence("Ctrl+Left"), this, SLOT(stepBackward()));
    new QShortcut(QKeySequence("Ctrl+Right"), this, SLOT(stepForward()));

    // Initial state
    updateSimulationControls();
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
    m_speedLabel->setText(tr("Speed: %1 ms").arg(value));

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
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #FFF0F0;"  // Very light red
            "  color: #C00000;"             // Dark red
            "  border-top: 1px solid #FFCCCC;"  // Light red top border
            "  padding: 4px 8px;"
            "}"
        );
    } else {
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #F5F5F5;"  // Very light gray
            "  color: #333333;"             // Dark text
            "  border-top: 1px solid #BDBDBD;"  // Light gray top border
            "  padding: 4px 8px;"
            "}"
        );
    }
}

