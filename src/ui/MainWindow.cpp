#include "MainWindow.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create the Turing Machine model
    turingMachine = std::make_unique<TuringMachine>("Untitled");

    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::stepForward);
    simulationSpeed = 500; // Default speed: 500ms per step

    // Setup UI components
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    setupCentralWidget();
    createDockWindows();

    // Read settings
    readSettings();

    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    // Clean up resources
    delete simulationTimer;
}

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
    // Create menus
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
    // File toolbar
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);

    // Simulation toolbar
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

void MainWindow::setupCentralWidget()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    tapeWidget = new TapeWidget(centralWidget);
    tapeWidget->setMinimumHeight(150);
    layout->addWidget(tapeWidget);

    tapeControlWidget = new TapeControlWidget(turingMachine->getTape(), tapeWidget, centralWidget);
    layout->addWidget(tapeControlWidget);

    tapeWidget->setTape(turingMachine->getTape());
}

void MainWindow::createDockWindows()
{
    // States dock
    statesDock = new QDockWidget(tr("States"), this);
    statesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // Create and add the states list widget directly as the dock widget
    StatesListWidget* statesWidget = new StatesListWidget(turingMachine.get(), this);
    statesDock->setWidget(statesWidget);
    addDockWidget(Qt::RightDockWidgetArea, statesDock);

    // Transitions dock
    transitionsDock = new QDockWidget(tr("Transitions"), this);
    transitionsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // Create and add the transitions list widget directly as the dock widget
    TransitionsListWidget* transitionsWidget = new TransitionsListWidget(turingMachine.get(), this);
    transitionsDock->setWidget(transitionsWidget);
    addDockWidget(Qt::RightDockWidgetArea, transitionsDock);

    // Properties dock
    propertiesDock = new QDockWidget(tr("Properties"), this);
    propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

    QLabel *propertiesLabel = new QLabel(tr("Properties Editor will go here"), this);
    propertiesLabel->setAlignment(Qt::AlignCenter);
    propertiesLabel->setStyleSheet("background-color: #e0e0e0; border: 1px solid #cccccc;");
    propertiesDock->setWidget(propertiesLabel);
    addDockWidget(Qt::BottomDockWidgetArea, propertiesDock);

    // Add dock widgets to view menu
    viewMenu->addAction(statesDock->toggleViewAction());
    viewMenu->addAction(transitionsDock->toggleViewAction());
    viewMenu->addAction(propertiesDock->toggleViewAction());

    // Connect signals
    connect(statesWidget, &StatesListWidget::stateAdded, transitionsWidget, &TransitionsListWidget::refreshTransitionsList);
    connect(statesWidget, &StatesListWidget::stateEdited, transitionsWidget, &TransitionsListWidget::refreshTransitionsList);
    connect(statesWidget, &StatesListWidget::stateRemoved, transitionsWidget, &TransitionsListWidget::refreshTransitionsList);
}

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

void MainWindow::closeEvent(QCloseEvent *event)
{
    // TODO: Replace with proper dirty flag tracking
    bool hasUnsavedChanges = (turingMachine->getStepCount() > 0); // Simple check for now

    if (hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Do you want to save before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            saveMachine();
            if (turingMachine->getName() == "Untitled") { // If still untitled, trigger Save As
                saveAsOperation();
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    writeSettings();
    event->accept();
}

void MainWindow::newMachine()
{
    // Create a new Turing Machine
    turingMachine = std::make_unique<TuringMachine>("Untitled");

    // Update tape-related widgets
    tapeWidget->setTape(turingMachine->getTape());
    tapeControlWidget->setTape(turingMachine->getTape());
    tapeControlWidget->resetTape();

    // Find and update states widget - more directly without using findChild
    StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
    if (statesWidget) {
        statesWidget->setMachine(turingMachine.get());
    }

    // Find and update transitions widget - more directly without using findChild
    TransitionsListWidget* transitionsWidget = qobject_cast<TransitionsListWidget*>(transitionsDock->widget());
    if (transitionsWidget) {
        transitionsWidget->setMachine(turingMachine.get());
    }

    // Reset UI state
    runAction->setEnabled(true);
    pauseAction->setEnabled(false);
    stepForwardAction->setEnabled(true);
    stepBackwardAction->setEnabled(false);

    // Reset the current file name
    currentFileName.clear();

    statusBar()->showMessage(tr("Created new machine"), 2000);
}

void MainWindow::openMachine()
{
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
        // Create new machine from JSON
        auto loadedMachine = TuringMachine::fromJson(jsonStr);
        if (!loadedMachine) {
            throw std::runtime_error("Failed to create machine from file");
        }

        // Replace current machine with loaded one
        turingMachine = std::move(loadedMachine);

        // Set current file name
        currentFileName = fileName;

        // Update tape-related widgets
        tapeWidget->setTape(turingMachine->getTape());
        tapeControlWidget->setTape(turingMachine->getTape());
        tapeControlWidget->resetTape();

        // Find and update states widget - directly cast the widget
        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->setMachine(turingMachine.get());
        }

        // Find and update transitions widget - directly cast the widget
        TransitionsListWidget* transitionsWidget = qobject_cast<TransitionsListWidget*>(transitionsDock->widget());
        if (transitionsWidget) {
            transitionsWidget->setMachine(turingMachine.get());
        }

        // Update UI state
        runAction->setEnabled(true);
        pauseAction->setEnabled(false);
        stepForwardAction->setEnabled(true);
        stepBackwardAction->setEnabled(turingMachine->canStepBackward());

        statusBar()->showMessage(tr("Opened %1").arg(fileName), 2000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid file format: %1").arg(e.what()));

        // Create a new machine in case of error
        turingMachine = std::make_unique<TuringMachine>("Untitled");
        currentFileName.clear();

        // Update UI with the new empty machine
        tapeWidget->setTape(turingMachine->getTape());
        tapeControlWidget->setTape(turingMachine->getTape());
        tapeControlWidget->resetTape();

        StatesListWidget* statesWidget = qobject_cast<StatesListWidget*>(statesDock->widget());
        if (statesWidget) {
            statesWidget->setMachine(turingMachine.get());
        }

        TransitionsListWidget* transitionsWidget = qobject_cast<TransitionsListWidget*>(transitionsDock->widget());
        if (transitionsWidget) {
            transitionsWidget->setMachine(turingMachine.get());
        }
    }
}

void MainWindow::saveMachine()
{
    if (currentFileName.isEmpty()) {
        saveAsOperation();
        return;
    }

    QFile file(currentFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not save file."));
        return;
    }

    QTextStream out(&file);
    out << QString::fromStdString(turingMachine->toJson());
    file.close();

    statusBar()->showMessage(tr("Machine saved to %1").arg(currentFileName), 2000);
}

void MainWindow::saveAsOperation()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Turing Machine"), "",
        tr("Turing Machine Files (*.tm);;All Files (*)"));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not save file."));
        return;
    }

    QTextStream out(&file);
    out << QString::fromStdString(turingMachine->toJson());
    file.close();

    currentFileName = fileName;
    statusBar()->showMessage(tr("Saved as %1").arg(fileName), 2000);
}

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

// Simulation menu slots
void MainWindow::runSimulation()
{
    if (turingMachine->getStatus() == ExecutionStatus::HALTED_ACCEPT ||
        turingMachine->getStatus() == ExecutionStatus::HALTED_REJECT ||
        turingMachine->getStatus() == ExecutionStatus::ERROR) {
        statusBar()->showMessage(tr("Cannot run - machine has halted"), 2000);
        return;
        }

    // Update UI state
    runAction->setEnabled(false);
    pauseAction->setEnabled(true);
    stepForwardAction->setEnabled(false);
    stepBackwardAction->setEnabled(false);
    resetAction->setEnabled(true);

    // Start the simulation
    simulationTimer->start(simulationSpeed);
    statusBar()->showMessage(tr("Simulation running..."));
}

void MainWindow::pauseSimulation()
{
    simulationTimer->stop();

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

    StatesListWidget* statesWidget = statesDock->widget()->findChild<StatesListWidget*>();
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
        int newHeadPosition = turingMachine->getTape()->getHeadPosition();
        if (newHeadPosition > oldHeadPosition) {
            tapeWidget->animateHeadMovement(true);
        } else if (newHeadPosition < oldHeadPosition) {
            tapeWidget->animateHeadMovement(false);
        } else {
            tapeWidget->updateTapeDisplay();
        }

        StatesListWidget* statesWidget = statesDock->widget()->findChild<StatesListWidget*>();
        if (statesWidget) {
            statesWidget->highlightCurrentState(turingMachine->getCurrentState());
        }

        stepBackwardAction->setEnabled(true);
        statusBar()->showMessage(tr("Running - Step %1 (State: %2)")
            .arg(turingMachine->getStepCount())
            .arg(QString::fromStdString(turingMachine->getCurrentState())));
    } else {
        simulationTimer->stop(); // Stop if halted

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

        StatesListWidget* statesWidget = statesDock->widget()->findChild<StatesListWidget*>();
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

        StatesListWidget* statesWidget = statesDock->widget()->findChild<StatesListWidget*>();
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

// Help menu slots
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
           "<p>For more details, see the project documentation.</p>"));
    statusBar()->showMessage(tr("Help contents opened"), 2000);
}