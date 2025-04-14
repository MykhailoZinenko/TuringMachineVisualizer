#pragma once

#include <QWidget>
#include <memory>

// Forward declarations
class CodeDocument;
class QTextEdit;
class QLabel;
class QLineEdit;
class QCheckBox;
class QSyntaxHighlighter;

/**
 * View for editing Turing machine code with IDE-like features
 */
class CodeEditorView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param document The document to edit
     * @param parent The parent widget
     */
    explicit CodeEditorView(CodeDocument* document, QWidget* parent = nullptr);

    /**
     * Destructor
     */
    ~CodeEditorView();

    /**
     * Get the document being edited
     * @return The document
     */
    CodeDocument* getDocument() const { return m_document; }

    /**
     * Update the view from the document
     */
    void updateFromDocument();

signals:
    /**
     * Signal emitted when the view's content is modified
     */
    void viewModified();

    /**
     * Signal emitted when the view is saved
     */
    void viewSaved();

private slots:
    /**
     * Handle text changes in the editor
     */
    void onTextChanged();

    /**
     * Handle document save
     */
    void onDocumentSaved(const std::string& filePath);

    /**
     * Handle document content change
     */
    void onDocumentContentChanged();

    /**
     * Handle machine update
     */
    void onMachineUpdated();

    /**
     * Undo last edit
     */
    void undo();

    /**
     * Redo last undone edit
     */
    void redo();

    /**
     * Show the find widget
     */
    void showFindWidget();

    /**
     * Hide the find widget
     */
    void hideFindWidget();

    /**
     * Handle search text changes
     */
    void searchTextChanged();

    /**
     * Find next occurrence
     */
    void findNext();

    /**
     * Find previous occurrence
     */
    void findPrevious();

private:
    // Document being edited
    CodeDocument* m_document;

    // UI components
    QTextEdit* m_codeEditor;
    QLabel* m_statusLabel;

    // Search components
    QWidget* m_findWidget;
    QLineEdit* m_searchField;
    QCheckBox* m_caseSensitiveCheckBox;
    int m_lastSearchPos = -1;

    // Syntax highlighter
    QSyntaxHighlighter* m_syntaxHighlighter;

    // Flag to prevent recursive updates
    bool m_ignoreTextChanges;

    /**
     * Set up the UI
     */
    void setupUI();

    /**
     * Set up the search widget
     */
    void setupSearchWidget();

    /**
     * Create the syntax highlighter
     */
    void createSyntaxHighlighter();

    /**
     * Set the status message
     * @param message The message to display
     * @param isError Whether the message is an error
     */
    void setStatusMessage(const QString& message, bool isError = false);

    /**
     * Count the occurrences of a search term
     * @param text The text to find
     * @return Number of occurrences
     */
    int countOccurrences(const QString& text);

    /**
     * Get the index of the current match
     * @param text The text to find
     * @return Index of current match (0-based)
     */
    int getCurrentMatchIndex(const QString& text);
};