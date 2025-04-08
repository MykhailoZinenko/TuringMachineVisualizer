#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include "model/TuringMachine.h"
#include "ui/StatesListWidget.h"
#include "ui/TransitionsListWidget.h"
#include "ui/TapeWidget.h"
#include "ui/TapeControlWidget.h"
#include "ui/PreferencesDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File menu actions
    void newMachine();
    void openMachine();
    void saveMachine();
    void saveAsOperation();

    // Edit menu actions
    void editPreferences();

    // Simulation menu actions
    void runSimulation();
    void pauseSimulation();
    void stepForward();
    void stepBackward();
    void resetSimulation();

    // Help menu actions
    void showAboutDialog();
    void showHelpContents();

private:
    // Data model
    std::unique_ptr<TuringMachine> turingMachine;

    // UI Components
    TapeWidget* tapeWidget;
    TapeControlWidget* tapeControlWidget;
    // Setup methods
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void setupCentralWidget();
    void readSettings();
    void writeSettings();

    // Event overrides
    void closeEvent(QCloseEvent *event) override;

    // UI Components
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *simulationMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *simulationToolBar;

    // File actions
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;

    // Edit actions
    QAction *preferencesAction;

    // Simulation actions
    QAction *runAction;
    QAction *pauseAction;
    QAction *stepForwardAction;
    QAction *stepBackwardAction;
    QAction *resetAction;

    QTimer* simulationTimer;
    int simulationSpeed;

    // Help actions
    QAction *aboutAction;
    QAction *helpAction;

    // Dock widgets
    QDockWidget *statesDock;
    QDockWidget *transitionsDock;
    QDockWidget *propertiesDock;

    QString currentFileName;
};

#endif // MAINWINDOW_H