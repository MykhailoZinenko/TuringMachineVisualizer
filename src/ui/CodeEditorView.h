#pragma once

#include <QWidget>
#include <memory>

// Forward declarations
class CodeDocument;
class QTextEdit;
class QLabel;
class QSyntaxHighlighter;

/**
 * View for editing Turing machine code
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

private:
    // Document being edited
    CodeDocument* m_document;

    // UI components
    QTextEdit* m_codeEditor;
    QLabel* m_statusLabel;

    // Syntax highlighter
    QSyntaxHighlighter* m_syntaxHighlighter;

    // Flag to prevent recursive updates
    bool m_ignoreTextChanges;

    /**
     * Set up the UI
     */
    void setupUI();

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
};