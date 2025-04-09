#pragma once

#include <QMainWindow>
#include <memory>
#include <string>

// Forward declarations
class QAction;
class QMenu;
class QToolBar;
class QStatusBar;
class QDockWidget;
class QTimer;

class TuringMachine;
class TapeWidget;
class TapeControlWidget;
class StatesListWidget;
class TransitionsListWidget;
class PropertiesEditorWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor & destructor
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Event handlers
    void closeEvent(QCloseEvent *event) override;

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

    // State & transition handling
    void handleStateAdded(const std::string& stateId);
    void handleStateEdited(const std::string& stateId);
    void handleStateRemoved(const std::string& stateId);
    void onStateSelected(const std::string& stateId);

    void handleTransitionAdded();
    void handleTransitionEdited();
    void handleTransitionRemoved();
    void onTransitionSelected(const std::string& fromState, char readSymbol);

    // Property changes
    void onMachinePropertiesChanged();
    void onStatePropertiesChanged(const std::string& stateId);
    void onTransitionPropertiesChanged(const std::string& fromState, char readSymbol);

    // Tape interaction
    void handleTapeContentChanged();
    void onCellValueChanged(int position, char newValue);
    void onHeadPositionChanged(int newPosition);

private:
    // Core data
    std::unique_ptr<TuringMachine> turingMachine;
    QString currentFileName;
    bool isDirty;

    // Simulation control
    QTimer* simulationTimer;
    int simulationSpeed;

    // UI components - Widgets
    TapeWidget* tapeWidget;
    TapeControlWidget* tapeControlWidget;
    PropertiesEditorWidget* propertiesEditor;

    // UI components - Docks
    QDockWidget *statesDock;
    QDockWidget *transitionsDock;
    QDockWidget *propertiesDock;

    // UI components - Menus
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *simulationMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    // UI components - Toolbars
    QToolBar *fileToolBar;
    QToolBar *simulationToolBar;

    // UI components - Actions
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;
    QAction *preferencesAction;
    QAction *runAction;
    QAction *pauseAction;
    QAction *stepForwardAction;
    QAction *stepBackwardAction;
    QAction *resetAction;
    QAction *aboutAction;
    QAction *helpAction;

    // Setup methods
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void setupCentralWidget();

    // Settings management
    void readSettings();
    void writeSettings();

    // UI state methods
    void setDirty(bool dirty = true);
    void updateWindowTitle();
};