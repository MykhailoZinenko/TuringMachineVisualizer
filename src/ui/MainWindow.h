#pragma once

#include <QMainWindow>
#include <memory>

class DocumentTabManager;
class QAction;
class QMenu;
class QToolBar;
class Project;
class Document;

/**
 * Main application window
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

    private slots:
        // File menu actions
        void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();

    // Tab handling
    void onDocumentTabChanged(Document* document);
    void onDocumentTabClosed(Document* document);

private:
    // UI components
    DocumentTabManager* m_tabManager;

    // Menus
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;

    // Toolbars
    QToolBar* m_fileToolBar;

    // Actions
    QAction* m_newProjectAction;
    QAction* m_openProjectAction;
    QAction* m_saveProjectAction;
    QAction* m_saveAsProjectAction;
    QAction* m_exitAction;

    // Current document and project
    Document* m_currentDocument;
    Project* m_currentProject;

    // Setup methods
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();

    // Settings
    void readSettings();
    void writeSettings();

    // Update UI
    void updateWindowTitle();
    void updateUIForDocument(Document* document);
};