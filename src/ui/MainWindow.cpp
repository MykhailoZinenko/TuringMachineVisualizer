#include "MainWindow.h"

// Qt includes
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <QCheckBox>
#include <QTabWidget>

// Project includes
#include "../model/TuringMachine.h"
#include "StatesListWidget.h"
#include "TransitionsListWidget.h"
#include "TapeWidget.h"
#include "TapeControlWidget.h"
#include "PreferencesDialog.h"
#include "PropertiesEditorWidget.h"
#include "CodeEditorWidget.h"

// Constructor & destructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isDirty(false)
{
    turingMachine = std::make_unique<TuringMachine>("Untitled");

    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::stepForward);
    simulationSpeed = 500; // Default speed: 500ms per step

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    setupCentralWidget();
    createDockWindows();

    readSettings();
    updateWindowTitle();

    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    delete simulationTimer;
}

// Setup central widget with tabs
void MainWindow::setupCentralWidget()
{
    // Create a tab widget as the central widget
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // Create the visual representation tab
    QWidget *visualTab = new QWidget(tabWidget);
    QVBoxLayout *visualLayout = new QVBoxLayout(visualTab);

    // Create tape widget and control widget for visual representation
    tapeWidget = new TapeWidget(visualTab);
    tapeWidget->setMinimumHeight(150);
    visualLayout->addWidget(tapeWidget);

    tapeControlWidget = new TapeControlWidget(turingMachine->getTape(), tapeWidget, visualTab);
    visualLayout->addWidget(tapeControlWidget);

    tapeWidget->setTape(turingMachine->getTape());

    connect(tapeControlWidget, &TapeControlWidget::tapeContentChanged, this, &MainWindow::handleTapeContentChanged);
    connect(tapeWidget, &TapeWidget::cellValueChanged, this, &MainWindow::onCellValueChanged);
    connect(tapeWidget, &TapeWidget::headPositionChanged, this, &MainWindow::onHeadPositionChanged);

    // Add the visual tab to the tab widget
    tabWidget->addTab(visualTab, tr("Visual"));

    // Create code editor tab
    codeEditorWidget = new CodeEditorWidget(turingMachine.get(), this);
    tabWidget->addTab(codeEditorWidget, tr("Code"));

    // Connect code editor signals
    connect(codeEditorWidget, &CodeEditorWidget::codeChanged, this, &MainWindow::handleCodeChanged);

    // Connect tab change signal
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleTabChanged);
}

// Handle tab changes
void MainWindow::handleTabChanged(int index)
{
    // Update the current representation based on the selected tab
    if (index == 0) { // Visual tab
        // Apply code changes to the model before updating the visual representation
        if (codeEditorWidget) {
            codeEditorWidget->applyCodeChanges();
        }
        // Update visual representation from model
        updateVisualFromModel();
    } else if (index == 1) { // Code tab
        // The code is the source of truth - we don't update it from the model
        // The code stays as the user wrote it
    }
}

// Update visual representation from model
void MainWindow::updateVisualFromModel()
{
    // Refresh tape display
    if (tapeWidget) {
        tapeWidget->setTape(turingMachine->getTape());
        tapeWidget->updateTapeDisplay();
    }

    // Refresh state and transition lists
    if (statesDock && transitionsDock) {
        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->setMachine(turingMachine.get());
            statesWidget->refreshStatesList();
        }

        TransitionsListWidget* transitionsWidget = qobject_cast<TransitionsListWidget*>(transitionsDock->widget());
        if (transitionsWidget) {
            transitionsWidget->setMachine(turingMachine.get());
            transitionsWidget->refreshTransitionsList();
        }
    }
}

// Handle code changes from code editor
void MainWindow::handleCodeChanged()
{
    // The code editor has updated the model
    setDirty(true);

    // If we're on the visual tab, update it to reflect changes
    if (tabWidget->currentIndex() == 0) {
        updateVisualFromModel();
    }
}

// Event handlers
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isDirty) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Do you want to save before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            saveMachine();
            if (isDirty) {
                // User canceled save operation
                event->ignore();
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    writeSettings();
    event->accept();
}

// File menu actions
// Fixed newMachine method for MainWindow.cpp
void MainWindow::newMachine()
{
    if (isDirty) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Do you want to save before creating a new machine?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            saveMachine();
            if (isDirty) {
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    // Create a new machine
    auto newTuringMachine = std::make_unique<TuringMachine>("Untitled");

    // Check if creation was successful
    if (!newTuringMachine) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create a new machine"));
        return;
    }

    // Add a default start state if needed
    if (newTuringMachine->getAllStates().empty()) {
        newTuringMachine->addState("q0", "Start State", StateType::START);
    }

    // Set default original code for a new machine
    std::string defaultCode = "// Untitled Machine\n";
    newTuringMachine->setOriginalCode(defaultCode);

    // Set the machine in a safe way
    turingMachine = std::move(newTuringMachine);
    currentFileName.clear();

    // Update tape
    if (tapeWidget) {
        tapeWidget->setTape(turingMachine->getTape());
        tapeWidget->updateTapeDisplay();
    }

    if (tapeControlWidget) {
        tapeControlWidget->setTape(turingMachine->getTape());
    }

    // Update UI components in a controlled sequence
    if (statesDock) {
        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->setMachine(turingMachine.get());
            statesWidget->refreshStatesList();
        }
    }

    if (transitionsDock) {
        TransitionsListWidget* transitionsWidget = qobject_cast<TransitionsListWidget*>(transitionsDock->widget());
        if (transitionsWidget) {
            transitionsWidget->setMachine(turingMachine.get());
            transitionsWidget->refreshTransitionsList();
        }
    }

    if (propertiesEditor) {
        propertiesEditor->setMachine(turingMachine.get());
    }

    // Switch to visual tab first
    if (tabWidget) {
        tabWidget->setCurrentIndex(0); // Visual tab
    }

    // Update code editor with a delay to ensure all other components are ready
    if (codeEditorWidget) {
        codeEditorWidget->setMachine(turingMachine.get());

        // Use a timer to defer code editor update
        QTimer::singleShot(100, this, [this]() {
            try {
                codeEditorWidget->updateFromModel();
            } catch (const std::exception& e) {
                qWarning() << "Error updating code editor:" << e.what();
            }
        });
    }

    // Update UI state
    runAction->setEnabled(true);
    pauseAction->setEnabled(false);
    stepForwardAction->setEnabled(true);
    stepBackwardAction->setEnabled(false);
    resetAction->setEnabled(true);

    setDirty(false);

    // Update window title
    updateWindowTitle();

    statusBar()->showMessage(tr("Created new machine"), 2000);
}

void MainWindow::openMachine() {
    if (isDirty) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Do you want to save before opening another machine?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            saveMachine();
            if (isDirty) {
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Turing Machine"), "",
        tr("Turing Machine Files (*.tm);;All Files (*)"));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file."));
        return;
    }

    QTextStream in(&file);
    std::string jsonStr = in.readAll().toStdString();
    file.close();

    try {
        qDebug() << "Loading machine from JSON file" << fileName;

        // Create new machine from the JSON file
        auto loadedMachine = TuringMachine::fromJson(jsonStr);
        if (!loadedMachine) {
            throw std::runtime_error("Failed to create machine from file");
        }

        // Log the number of transitions loaded
        qDebug() << "Loaded machine with" << loadedMachine->getAllTransitions().size() << "transitions";
        qDebug() << "Loaded original code size:" << loadedMachine->getOriginalCode().size() << "bytes";

        // Store the new machine in the class member
        turingMachine = std::move(loadedMachine);
        currentFileName = fileName;

        // First update the tape and core components
        if (tapeWidget) {
            tapeWidget->setTape(turingMachine->getTape());
            tapeWidget->updateTapeDisplay();
        }

        if (tapeControlWidget) {
            tapeControlWidget->setTape(turingMachine->getTape());
        }

        // Update all the UI components with the new machine reference
        updateVisualFromModel();

        // Create a separate code editor update to avoid synchronization issues
        if (codeEditorWidget) {
            // Make sure the code editor has the right machine pointer
            codeEditorWidget->setMachine(turingMachine.get());

            // Use a timer to ensure UI is updated before we generate code
            QTimer::singleShot(100, this, [this]() {
                qDebug() << "Deferred code editor update - machine has"
                         << turingMachine->getAllTransitions().size() << "transitions";

                // Force a complete refresh of the code editor
                codeEditorWidget->updateFromModel();
            });
        }

        // Update UI state
        runAction->setEnabled(true);
        pauseAction->setEnabled(false);
        stepForwardAction->setEnabled(true);
        stepBackwardAction->setEnabled(turingMachine->canStepBackward());

        setDirty(false);
        statusBar()->showMessage(tr("Opened %1").arg(fileName), 2000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid file format: %1").arg(e.what()));

        turingMachine = std::make_unique<TuringMachine>("Untitled");
        currentFileName.clear();

        updateVisualFromModel();

        if (codeEditorWidget) {
            codeEditorWidget->setMachine(turingMachine.get());
            codeEditorWidget->updateFromModel();
        }

        setDirty(false);
    }
}

void MainWindow::saveMachine()
{
    if (currentFileName.isEmpty()) {
        saveAsOperation();
        return;
    }

    // Make sure we capture any code changes before saving
    if (codeEditorWidget && tabWidget->currentIndex() == 1) {
        // We're on the code tab, apply changes to ensure the latest code is saved
        codeEditorWidget->applyCodeChanges();
    }

    QFile file(currentFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not save file."));
        return;
    }

    std::string jsonStr = turingMachine->toJson();
    QTextStream out(&file);
    out << QString::fromStdString(jsonStr);
    file.close();

    setDirty(false);
    statusBar()->showMessage(tr("Machine saved to %1").arg(currentFileName), 2000);
}

void MainWindow::saveAsOperation()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Turing Machine"), "",
        tr("Turing Machine Files (*.tm);;All Files (*)"));

    if (fileName.isEmpty()) return;

    if (!fileName.endsWith(".tm", Qt::CaseInsensitive)) {
        fileName += ".tm";
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not save file."));
        return;
    }

    std::string jsonStr = turingMachine->toJson();
    QTextStream out(&file);
    out << QString::fromStdString(jsonStr);
    file.close();

    currentFileName = fileName;
    setDirty(false);
    statusBar()->showMessage(tr("Saved as %1").arg(fileName), 2000);
}

// Edit menu actions
void MainWindow::editPreferences()
{
    PreferencesDialog dialog(simulationSpeed, this);
    if (dialog.exec() == QDialog::Accepted) {
        simulationSpeed = dialog.getSimulationSpeed();
        if (simulationTimer->isActive()) {
            simulationTimer->setInterval(simulationSpeed);
        }
        statusBar()->showMessage(tr("Preferences updated"), 2000);
    }
}

// Simulation menu actions
void MainWindow::runSimulation()
{
    if (turingMachine->getStatus() == ExecutionStatus::HALTED_ACCEPT ||
        turingMachine->getStatus() == ExecutionStatus::HALTED_REJECT ||
        turingMachine->getStatus() == ExecutionStatus::ERROR) {
        statusBar()->showMessage(tr("Cannot run - machine has halted"), 2000);
        return;
    }

    tapeWidget->setInteractiveMode(false);
    if (tapeControlWidget) {
        QCheckBox* interactiveModeCheckbox = tapeControlWidget->findChild<QCheckBox*>();
        if (interactiveModeCheckbox) {
            interactiveModeCheckbox->setChecked(false);
            interactiveModeCheckbox->setEnabled(false);
        }
    }

    runAction->setEnabled(false);
    pauseAction->setEnabled(true);
    stepForwardAction->setEnabled(false);
    stepBackwardAction->setEnabled(false);
    resetAction->setEnabled(true);

    simulationTimer->start(simulationSpeed);
    statusBar()->showMessage(tr("Simulation running..."));
}

void MainWindow::pauseSimulation()
{
    simulationTimer->stop();

    if (tapeControlWidget) {
        QCheckBox* interactiveModeCheckbox = tapeControlWidget->findChild<QCheckBox*>();
        if (interactiveModeCheckbox) {
            interactiveModeCheckbox->setEnabled(true);
        }
    }

    runAction->setEnabled(true);
    pauseAction->setEnabled(false);
    stepForwardAction->setEnabled(true);
    stepBackwardAction->setEnabled(turingMachine->canStepBackward());

    statusBar()->showMessage(tr("Simulation paused"), 2000);
}

void MainWindow::resetSimulation()
{
    simulationTimer->stop();
    turingMachine->reset();
    tapeWidget->updateTapeDisplay();

    tapeWidget->setInteractiveMode(true);
    if (tapeControlWidget) {
        QCheckBox* interactiveModeCheckbox = tapeControlWidget->findChild<QCheckBox*>();
        if (interactiveModeCheckbox) {
            interactiveModeCheckbox->setChecked(true);
            interactiveModeCheckbox->setEnabled(true);
        }
    }

    StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
    if (statesWidget) {
        statesWidget->highlightCurrentState(turingMachine->getCurrentState());
    }

    runAction->setEnabled(true);
    pauseAction->setEnabled(false);
    stepForwardAction->setEnabled(true);
    stepBackwardAction->setEnabled(false);

    statusBar()->showMessage(tr("Simulation reset"), 2000);
}

void MainWindow::stepForward()
{
    int oldHeadPosition = turingMachine->getTape()->getHeadPosition();

    if (turingMachine->step()) {
        setDirty();

        int newHeadPosition = turingMachine->getTape()->getHeadPosition();
        if (newHeadPosition > oldHeadPosition) {
            tapeWidget->animateHeadMovement(true);
        } else if (newHeadPosition < oldHeadPosition) {
            tapeWidget->animateHeadMovement(false);
        } else {
            tapeWidget->updateTapeDisplay();
        }

        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->highlightCurrentState(turingMachine->getCurrentState());
        }

        stepBackwardAction->setEnabled(true);
        statusBar()->showMessage(tr("Running - Step %1 (State: %2)")
            .arg(turingMachine->getStepCount())
            .arg(QString::fromStdString(turingMachine->getCurrentState())));
    } else {
        simulationTimer->stop();

        ExecutionStatus status = turingMachine->getStatus();
        QString message;
        switch (status) {
            case ExecutionStatus::HALTED_ACCEPT:
                message = tr("Halted - Accept State (Step %1)")
                    .arg(turingMachine->getStepCount());
            break;
            case ExecutionStatus::HALTED_REJECT:
                message = tr("Halted - Reject State (Step %1)")
                    .arg(turingMachine->getStepCount());
            break;
            case ExecutionStatus::ERROR:
                message = tr("Halted - No valid transition (Step %1)")
                    .arg(turingMachine->getStepCount());
            break;
            default:
                message = tr("Halted unexpectedly (Step %1)")
                    .arg(turingMachine->getStepCount());
        }

        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->highlightCurrentState(turingMachine->getCurrentState());
        }

        runAction->setEnabled(false);
        pauseAction->setEnabled(false);
        stepForwardAction->setEnabled(false);
        stepBackwardAction->setEnabled(turingMachine->canStepBackward());
        statusBar()->showMessage(message);
    }
}

void MainWindow::stepBackward()
{
    int oldHeadPosition = turingMachine->getTape()->getHeadPosition();

    if (turingMachine->stepBackward()) {
        int newHeadPosition = turingMachine->getTape()->getHeadPosition();
        if (newHeadPosition > oldHeadPosition) {
            tapeWidget->animateHeadMovement(true);
        } else if (newHeadPosition < oldHeadPosition) {
            tapeWidget->animateHeadMovement(false);
        } else {
            tapeWidget->updateTapeDisplay();
        }

        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->highlightCurrentState(turingMachine->getCurrentState());
        }

        statusBar()->showMessage(tr("Step %1 (State: %2)")
            .arg(turingMachine->getStepCount())
            .arg(QString::fromStdString(turingMachine->getCurrentState())));
        stepBackwardAction->setEnabled(turingMachine->canStepBackward());
    } else {
        statusBar()->showMessage(tr("Cannot step backward further"), 2000);
        stepBackwardAction->setEnabled(false);
    }
}

// Help menu actions
void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("About Turing Machine Visualizer"),
        tr("<h2>Turing Machine Visualizer</h2>"
           "<p>Version 0.1</p>"
           "<p>A visual simulator for Turing machines.</p>"
           "<p>Created by Your Name</p>"));
}

void MainWindow::showHelpContents()
{
    QMessageBox::information(this, tr("Help Contents"),
        tr("<h2>Turing Machine Visualizer Help</h2>"
           "<p><b>File Menu:</b> Create, open, and save machines.</p>"
           "<p><b>Simulation Menu:</b> Run, pause, step through, or reset the simulation.</p>"
           "<p><b>Tape Controls:</b> Set initial tape content and adjust speed.</p>"
           "<p><b>States/Transitions:</b> Add, edit, or remove states and transitions.</p>"
           "<p><b>Views:</b> Switch between Visual, Code, and Graph representations.</p>"
           "<p>For more details, see the project documentation.</p>"));
    statusBar()->showMessage(tr("Help contents opened"), 2000);
}

// State & transition handling
void MainWindow::handleStateAdded(const std::string& stateId)
{
    setDirty();

    // Update other views if needed
    if (tabWidget->currentIndex() == 1 && codeEditorWidget) {
        codeEditorWidget->updateFromModel();
    }
}

void MainWindow::handleStateEdited(const std::string& stateId)
{
    setDirty();

    // Update other views if needed
    if (tabWidget->currentIndex() == 1 && codeEditorWidget) {
        codeEditorWidget->updateFromModel();
    }
}

void MainWindow::handleStateRemoved(const std::string& stateId)
{
    setDirty();

    // Update other views if needed
    if (tabWidget->currentIndex() == 1 && codeEditorWidget) {
        codeEditorWidget->updateFromModel();
    }
}

void MainWindow::onStateSelected(const std::string& stateId)
{
    if (propertiesEditor) {
        propertiesEditor->selectState(stateId);
    }
}

void MainWindow::handleTransitionAdded()
{
    setDirty();

    // If code editor exists, append the newly added transition
    if (codeEditorWidget) {
        // Find the newest transition
        auto transitions = turingMachine->getAllTransitions();
        if (!transitions.empty()) {
            // Get the last transition (most recently added)
            Transition* lastTransition = transitions.back();
            if (lastTransition) {
                // Append it to the code
                codeEditorWidget->appendTransition(lastTransition);
            }
        }
    }
}

void MainWindow::handleTransitionEdited()
{
    setDirty();

}

void MainWindow::handleTransitionRemoved()
{
    setDirty();

}

void MainWindow::onTransitionSelected(const std::string& fromState, const std::string& readSymbol)
{
    if (propertiesEditor) {
        propertiesEditor->selectTransition(fromState, readSymbol);
    }
}

// Property changes
void MainWindow::onMachinePropertiesChanged()
{
    updateWindowTitle();
    setDirty();

    // Update code editor if it's open
    if (codeEditorWidget && tabWidget->currentIndex() == 1) {
        codeEditorWidget->updateFromModel();
    }
}

void MainWindow::onStatePropertiesChanged(const std::string& stateId)
{
    StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
    if (statesWidget) {
        statesWidget->refreshStatesList();
        statesWidget->highlightCurrentState(turingMachine->getCurrentState());
    }

    setDirty();

    // Update code editor if it's open
    if (codeEditorWidget && tabWidget->currentIndex() == 1) {
        codeEditorWidget->updateFromModel();
    }
}

void MainWindow::onTransitionPropertiesChanged(const std::string& fromState, const std::string& readSymbol)
{
    TransitionsListWidget* transitionsWidget = qobject_cast<TransitionsListWidget*>(transitionsDock->widget());
    if (transitionsWidget) {
        transitionsWidget->refreshTransitionsList();
    }

    setDirty();
}

// Tape interaction
void MainWindow::handleTapeContentChanged()
{
    setDirty();
}

void MainWindow::onCellValueChanged(int position, const std::string& newValue)
{
    setDirty();
    statusBar()->showMessage(tr("Cell at position %1 changed to '%2'")
                            .arg(position)
                            .arg(QString::fromStdString(newValue)), 2000);
}

void MainWindow::onHeadPositionChanged(int newPosition)
{
    setDirty();

    StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
    if (statesWidget) {
        statesWidget->highlightCurrentState(turingMachine->getCurrentState());
    }

    statusBar()->showMessage(tr("Head moved to position %1").arg(newPosition), 2000);
}

// Setup methods
void MainWindow::createActions()
{
    // File actions
    newAction = new QAction(tr("&New"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new Turing machine"));
    connect(newAction, &QAction::triggered, this, &MainWindow::newMachine);

    openAction = new QAction(tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing Turing machine"));
    connect(openAction, &QAction::triggered, this, &MainWindow::openMachine);

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the current Turing machine"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveMachine);

    saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setShortcuts(QKeySequence::SaveAs);
    saveAsAction->setStatusTip(tr("Save the current Turing machine with a new name"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsOperation);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Edit actions
    preferencesAction = new QAction(tr("&Preferences..."), this);
    preferencesAction->setStatusTip(tr("Configure application settings"));
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::editPreferences);

    // Simulation actions
    runAction = new QAction(tr("&Run"), this);
    runAction->setStatusTip(tr("Run the Turing machine simulation"));
    connect(runAction, &QAction::triggered, this, &MainWindow::runSimulation);

    pauseAction = new QAction(tr("&Pause"), this);
    pauseAction->setStatusTip(tr("Pause the simulation"));
    pauseAction->setEnabled(false);
    connect(pauseAction, &QAction::triggered, this, &MainWindow::pauseSimulation);

    stepForwardAction = new QAction(tr("Step &Forward"), this);
    stepForwardAction->setStatusTip(tr("Execute one step forward"));
    connect(stepForwardAction, &QAction::triggered, this, &MainWindow::stepForward);

    stepBackwardAction = new QAction(tr("Step &Backward"), this);
    stepBackwardAction->setStatusTip(tr("Go one step backward"));
    stepBackwardAction->setEnabled(false);
    connect(stepBackwardAction, &QAction::triggered, this, &MainWindow::stepBackward);

    resetAction = new QAction(tr("&Reset"), this);
    resetAction->setStatusTip(tr("Reset the simulation to initial state"));
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetSimulation);

    // Help actions
    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    helpAction = new QAction(tr("&Help Contents"), this);
    helpAction->setShortcuts(QKeySequence::HelpContents);
    helpAction->setStatusTip(tr("Show the help contents"));
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelpContents);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(preferencesAction);

    simulationMenu = menuBar()->addMenu(tr("&Simulation"));
    simulationMenu->addAction(runAction);
    simulationMenu->addAction(pauseAction);
    simulationMenu->addAction(stepForwardAction);
    simulationMenu->addAction(stepBackwardAction);
    simulationMenu->addAction(resetAction);

    viewMenu = menuBar()->addMenu(tr("&View"));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAction);
    helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);

    simulationToolBar = addToolBar(tr("Simulation"));
    simulationToolBar->addAction(runAction);
    simulationToolBar->addAction(pauseAction);
    simulationToolBar->addAction(stepForwardAction);
    simulationToolBar->addAction(stepBackwardAction);
    simulationToolBar->addAction(resetAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows()
{
    // States dock
    statesDock = new QDockWidget(tr("States"), this);
    statesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    StatesListWidget* statesWidget = new StatesListWidget(turingMachine.get(), this);
    statesDock->setWidget(statesWidget);
    addDockWidget(Qt::RightDockWidgetArea, statesDock);

    // Transitions dock
    transitionsDock = new QDockWidget(tr("Transitions"), this);
    transitionsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    TransitionsListWidget* transitionsWidget = new TransitionsListWidget(turingMachine.get(), this);
    transitionsDock->setWidget(transitionsWidget);
    addDockWidget(Qt::RightDockWidgetArea, transitionsDock);

    // Properties dock
    propertiesDock = new QDockWidget(tr("Properties"), this);
    propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

    propertiesEditor = new PropertiesEditorWidget(turingMachine.get(), this);
    propertiesDock->setWidget(propertiesEditor);
    addDockWidget(Qt::BottomDockWidgetArea, propertiesDock);

    // Add dock widgets to view menu
    viewMenu->addAction(statesDock->toggleViewAction());
    viewMenu->addAction(transitionsDock->toggleViewAction());
    viewMenu->addAction(propertiesDock->toggleViewAction());

    // Connect signals
    connect(statesWidget, &StatesListWidget::stateAdded, transitionsWidget, &TransitionsListWidget::refreshTransitionsList);
    connect(statesWidget, &StatesListWidget::stateEdited, transitionsWidget, &TransitionsListWidget::refreshTransitionsList);
    connect(statesWidget, &StatesListWidget::stateRemoved, transitionsWidget, &TransitionsListWidget::refreshTransitionsList);

    connect(statesWidget, &StatesListWidget::stateAdded, this, &MainWindow::handleStateAdded);
    connect(statesWidget, &StatesListWidget::stateEdited, this, &MainWindow::handleStateEdited);
    connect(statesWidget, &StatesListWidget::stateRemoved, this, &MainWindow::handleStateRemoved);

    connect(transitionsWidget, &TransitionsListWidget::transitionAdded, this, &MainWindow::handleTransitionAdded);
    connect(transitionsWidget, &TransitionsListWidget::transitionEdited, this, &MainWindow::handleTransitionEdited);
    connect(transitionsWidget, &TransitionsListWidget::transitionRemoved, this, &MainWindow::handleTransitionRemoved);

    connect(statesWidget, &StatesListWidget::stateSelected, this, &MainWindow::onStateSelected);
    connect(transitionsWidget, &TransitionsListWidget::transitionSelected, this, &MainWindow::onTransitionSelected);

    connect(propertiesEditor, &PropertiesEditorWidget::machinePropertiesChanged, this, &MainWindow::onMachinePropertiesChanged);
    connect(propertiesEditor, &PropertiesEditorWidget::statePropertiesChanged, this, &MainWindow::onStatePropertiesChanged);
    connect(propertiesEditor, &PropertiesEditorWidget::transitionPropertiesChanged, this, &MainWindow::onTransitionPropertiesChanged);
}

// Settings management
void MainWindow::readSettings()
{
    QSettings settings("YourOrganization", "TuringMachineVisualizer");
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
        resize(availableGeometry.width() * 0.8, availableGeometry.height() * 0.7);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }

    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::writeSettings()
{
    QSettings settings("YourOrganization", "TuringMachineVisualizer");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

// UI state methods
void MainWindow::setDirty(bool dirty)
{
    if (isDirty != dirty) {
        isDirty = dirty;
        updateWindowTitle();
        saveAction->setEnabled(isDirty);
    }
}

void MainWindow::updateWindowTitle()
{
    QString title = turingMachine->getName().c_str();
    if (title.isEmpty()) {
        title = "Untitled";
    }

    if (isDirty) {
        title += " *";
    }

    setWindowTitle(title + " - Turing Machine Visualizer");
}