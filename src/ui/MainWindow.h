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
#include "ui/PropertiesEditorWidget.h"

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

    // Changes tracking
    void setDirty(bool dirty = true);
    void handleStateAdded(const std::string& stateId);
    void handleStateEdited(const std::string& stateId);
    void handleStateRemoved(const std::string& stateId);
    void handleTransitionAdded();
    void handleTransitionEdited();
    void handleTransitionRemoved();
    void handleTapeContentChanged();

    void onStateSelected(const std::string& stateId);
    void onTransitionSelected(const std::string& fromState, char readSymbol);
    void onMachinePropertiesChanged();
    void onStatePropertiesChanged(const std::string& stateId);
    void onTransitionPropertiesChanged(const std::string& fromState, char readSymbol);

    void onCellValueChanged(int position, char newValue);
    void onHeadPositionChanged(int newPosition);

private:
    // Data model
    std::unique_ptr<TuringMachine> turingMachine;

    // UI Components
    TapeWidget* tapeWidget;
    TapeControlWidget* tapeControlWidget;
    PropertiesEditorWidget* propertiesEditor;

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

    // Tracking unsaved changes
    bool isDirty;
    void updateWindowTitle();
};

#endif // MAINWINDOW_H