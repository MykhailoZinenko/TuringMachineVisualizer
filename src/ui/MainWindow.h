#pragma once

#include <QMainWindow>
#include <memory>

// Forward declarations
class QAction;
class QMenu;
class QToolBar;
class QSplitter;
class QDockWidget;
class FileListManager;
class CodeTabManager;
class TapeTabManager;
class CodeDocument;
class TapeDocument;

/**
 * Main application window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent Parent widget
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * Destructor
     */
    ~MainWindow();

protected:
    /**
     * Handle close event
     * @param event The close event
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File menu actions
    void newCodeFile();
    void newTapeFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exit();

    // Edit menu actions
    void cut();
    void copy();
    void paste();

    // View menu actions
    void toggleFileList();

    // Help menu actions
    void about();

    // File list actions
    void onCodeFileSelected(const QString& path);
    void onTapeFileSelected(const QString& path);
    void onCodeDocumentCreated(CodeDocument* document);
    void onTapeDocumentCreated(TapeDocument* document);

    // Tab manager actions
    void onCodeDocumentClosed(CodeDocument* document);
    void onTapeDocumentClosed(TapeDocument* document);
    void onActiveCodeDocumentChanged(CodeDocument* document);
    void onActiveTapeDocumentChanged(TapeDocument* document);

private:
    // Create actions, menus, and toolbars
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();

    // Create dock widgets and central widget
    void createDockWidgets();
    void createCentralWidget();

    // Read/write settings
    void readSettings();
    void writeSettings();

    // Update UI
    void updateWindowTitle();
    void updateActions();

    // Handle unsaved changes
    bool checkUnsavedChanges();

    // Member variables

    // Actions
    QAction* m_newCodeAction;
    QAction* m_newTapeAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_exitAction;

    QAction* m_cutAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;

    QAction* m_toggleFileListAction;

    QAction* m_aboutAction;

    // Menus
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;

    // Toolbars
    QToolBar* m_fileToolBar;
    QToolBar* m_editToolBar;

    // Dock widgets
    QDockWidget* m_fileListDock;

    // Central widget
    QSplitter* m_splitter;

    // Managers
    FileListManager* m_fileListManager;
    CodeTabManager* m_codeTabManager;
    TapeTabManager* m_tapeTabManager;
};