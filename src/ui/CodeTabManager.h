#pragma once

#include <QTabWidget>
#include <map>
#include <memory>

// Forward declarations
class CodeDocument;
class CodeEditorView;
class QMenu;

/**
 * Tab widget that manages code editor tabs
 */
class CodeTabManager : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent Parent widget
     */
    explicit CodeTabManager(QWidget* parent = nullptr);
    
    /**
     * Destructor
     */
    ~CodeTabManager();
    
    /**
     * Open a code document in a tab
     * @param document The document to open
     * @return True if opened successfully, false otherwise
     */
    bool openDocument(CodeDocument* document);
    
    /**
     * Check if a document is already open
     * @param document The document to check
     * @return True if the document is open, false otherwise
     */
    bool isDocumentOpen(CodeDocument* document) const;
    
    /**
     * Find a document by file path
     * @param filePath The file path to look for
     * @return The document if found, nullptr otherwise
     */
    CodeDocument* findDocumentByPath(const std::string& filePath) const;
    
    /**
     * Get the currently active document
     * @return The active document, or nullptr if none
     */
    CodeDocument* getActiveDocument() const;
    
    /**
     * Create a new document
     * @param filePath The file path for the new document
     * @return The new document if created successfully, nullptr otherwise
     */
    CodeDocument* createDocument(const std::string& filePath = "");
    
    /**
     * Close a document
     * @param document The document to close
     * @return True if closed successfully, false otherwise
     */
    bool closeDocument(CodeDocument* document);
    
    /**
     * Close the current tab
     * @return True if closed successfully, false otherwise
     */
    bool closeCurrentTab();
    
    /**
     * Save the current document
     * @return True if saved successfully, false otherwise
     */
    bool saveCurrentDocument();
    
    /**
     * Save the current document with a new file path
     * @return True if saved successfully, false otherwise
     */
    bool saveCurrentDocumentAs();

signals:
    /**
     * Signal emitted when the active document changes
     * @param document The new active document
     */
    void activeDocumentChanged(CodeDocument* document);
    
    /**
     * Signal emitted when a document is opened
     * @param document The opened document
     */
    void documentOpened(CodeDocument* document);
    
    /**
     * Signal emitted when a document is closed
     * @param document The closed document
     */
    void documentClosed(CodeDocument* document);
    
    /**
     * Signal emitted when a document is saved
     * @param document The saved document
     */
    void documentSaved(CodeDocument* document);

private slots:
    /**
     * Handle tab close request
     * @param index The index of the tab to close
     */
    void onTabCloseRequested(int index);
    
    /**
     * Handle current tab change
     * @param index The index of the new current tab
     */
    void onCurrentChanged(int index);
    
    /**
     * Handle tab context menu
     * @param point The point where the context menu was requested
     */
    void onTabContextMenu(const QPoint& point);
    
    /**
     * Handle document modified status change
     */
    void onDocumentModified();

private:
    // Map of documents to their views
    std::map<CodeDocument*, CodeEditorView*> m_documentViews;
    
    // Tab context menu
    QMenu* m_tabContextMenu;
    
    /**
     * Set up the context menu
     */
    void setupContextMenu();
    
    /**
     * Update tab text with modification indicator
     * @param view The view to update
     */
    void updateTabText(CodeEditorView* view);
    
    /**
     * Find tab index for a document
     * @param document The document to find
     * @return The tab index, or -1 if not found
     */
    int findTabIndex(CodeDocument* document) const;
    
    /**
     * Check if document has unsaved changes and prompt
     * @param document The document to check
     * @return True to proceed, false to cancel
     */
    bool checkUnsavedChanges(CodeDocument* document);
    
    /**
     * Create a view for a document
     * @param document The document
     * @return The created view
     */
    CodeEditorView* createViewForDocument(CodeDocument* document);
};