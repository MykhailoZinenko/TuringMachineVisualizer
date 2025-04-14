#include "CodeEditorView.h"
#include "TuringMachineSyntaxHighlighter.h"
#include "../document/CodeDocument.h"
#include "../model/TuringMachine.h"

#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QTextCursor>
#include <QScrollBar>
#include <QDebug>

#include "src/core/SessionManager.h"

CodeEditorView::CodeEditorView(CodeDocument* document, QWidget* parent)
    : QWidget(parent), m_document(document), m_ignoreTextChanges(false)
{
    setupUI();
    createSyntaxHighlighter();

    if (m_document) {
        updateFromDocument();

        // Connect document signals
        connect(m_document, &Document::contentChanged,
                this, &CodeEditorView::onDocumentContentChanged);
        connect(m_document, &Document::documentSaved,
                this, &CodeEditorView::onDocumentSaved);
        connect(m_document, &CodeDocument::machineUpdated,
                this, &CodeEditorView::onMachineUpdated);
    }
}

CodeEditorView::~CodeEditorView()
{
    // The syntax highlighter is automatically deleted when the document is
}

void CodeEditorView::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create code editor
    m_codeEditor = new QTextEdit(this);
    m_codeEditor->setLineWrapMode(QTextEdit::NoWrap);

    // Set monospace font
    QFont font("Courier New", 10);
    font.setStyleHint(QFont::Monospace);
    m_codeEditor->setFont(font);

    // Set tab width to 4 spaces
    QFontMetrics metrics(font);
    m_codeEditor->setTabStopDistance(4 * metrics.horizontalAdvance(' '));

    // Status label
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_statusLabel->setMinimumHeight(24);

    // Add to layout
    layout->addWidget(m_codeEditor, 1);
    layout->addWidget(m_statusLabel);

    // Connect signals
    connect(m_codeEditor, &QTextEdit::textChanged, this, &CodeEditorView::onTextChanged);
}

void CodeEditorView::createSyntaxHighlighter()
{
    m_syntaxHighlighter = new TuringMachineSyntaxHighlighter(m_codeEditor->document());
}

void CodeEditorView::updateFromDocument()
{
    if (!m_document) {
        return;
    }

    // Save cursor and scroll position
    QTextCursor cursor = m_codeEditor->textCursor();
    int position = cursor.position();
    int scrollValue = m_codeEditor->verticalScrollBar()->value();

    // Update editor content
    m_ignoreTextChanges = true;
    m_codeEditor->setPlainText(QString::fromStdString(m_document->getCode()));
    m_ignoreTextChanges = false;

    // Restore cursor position
    if (position <= m_codeEditor->document()->characterCount()) {
        cursor.setPosition(position);
        m_codeEditor->setTextCursor(cursor);
    }

    // Restore scroll position
    m_codeEditor->verticalScrollBar()->setValue(scrollValue);

    // Update status
    setStatusMessage(tr("Document loaded"));
}

void CodeEditorView::onTextChanged()
{
    if (m_ignoreTextChanges || !m_document) {
        return;
    }

    // Update document with editor content
    std::string code = m_codeEditor->toPlainText().toStdString();

    // Don't call updateMachine here; that happens when file is saved
    m_document->setCode(code, false);

    // Notify that the view content has changed
    emit viewModified();
}

void CodeEditorView::onDocumentSaved(const std::string& filePath)
{
    Q_UNUSED(filePath);

    if (m_document) {
        // Update status
        setStatusMessage(tr("Document saved"));

        // This is where the machine update happens, when file is saved
        bool success = m_document->updateMachine();

        if (success) {
            setStatusMessage(tr("Document saved and machine updated"));

            // Make sure the active machine is properly updated in SessionManager
            TuringMachine* machine = m_document->getMachine();
            if (machine) {
                auto& sessionManager = SessionManager::getInstance();
                if (sessionManager.getActiveCodeDocument() == m_document) {
                    sessionManager.activeMachineUpdated(machine);
                }
            }
        } else {
            setStatusMessage(tr("Document saved but machine update failed"), true);
        }

        // Notify that the view has been saved
        emit viewSaved();
    }
}

void CodeEditorView::onDocumentContentChanged()
{
    // This slot is triggered when the document content changes from outside
    // For example, when the file is reloaded from disk

    // We call updateFromDocument which will preserve cursor position
    updateFromDocument();
}

void CodeEditorView::onMachineUpdated()
{
    if (m_document && m_document->getMachine()) {
        TuringMachine* machine = m_document->getMachine();

        // Update status with machine info
        int states = machine->getAllStates().size();
        int transitions = machine->getAllTransitions().size();

        setStatusMessage(tr("Machine updated: %1 states, %2 transitions")
                        .arg(states)
                        .arg(transitions));
    }
}

void CodeEditorView::setStatusMessage(const QString& message, bool isError)
{
    m_statusLabel->setText(message);
    
    if (isError) {
        m_statusLabel->setStyleSheet("color: red;");
    } else {
        m_statusLabel->setStyleSheet("");
    }
}