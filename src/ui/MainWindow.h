#pragma once

#include <QMainWindow>
#include <memory>

class DocumentTabManager;
class Document;
class QAction;
class QMenu;
class QToolBar;

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
        void newCodeDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();

    // Document handling
    void onDocumentTabChanged(Document* document);
    void onDocumentTabClosed(Document* document);
    void onDocumentAdded(Document* document);

private:
    // UI components
    DocumentTabManager* m_tabManager;

    // Menus
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_simulationMenu;
    QMenu* m_helpMenu;

    // Toolbars
    QToolBar* m_fileToolBar;

    // Actions
    QAction* m_newCodeAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_exitAction;

    // Current document
    Document* m_currentDocument;

    // Setup methods
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();

    // Settings
    void readSettings();
    void writeSettings();

    // Update UI based on current document
    void updateUIForDocument(Document* document);
};