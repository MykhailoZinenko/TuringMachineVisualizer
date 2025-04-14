#include "CodeEditorView.h"
#include "StyleKit.h"
#include "TuringMachineSyntaxHighlighter.h"
#include "../document/CodeDocument.h"
#include "../model/TuringMachine.h"

#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QToolButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QShortcut>
#include <QFont>
#include <QTextCursor>
#include <QScrollBar>
#include <QDebug>
#include <QPushButton>

#include "src/core/SessionManager.h"

CodeEditorView::CodeEditorView(CodeDocument* document, QWidget* parent)
    : QWidget(parent), m_document(document), m_ignoreTextChanges(false),
      m_findWidget(nullptr), m_searchField(nullptr)
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
    layout->setSpacing(0);

    // Create editor toolbar with IDE style
    QToolBar* editorToolbar = new QToolBar(this);
    editorToolbar->setIconSize(QSize(16, 16));
    editorToolbar->setStyleSheet(StyleKit::getToolbarStyle());

    // Create toolbar actions
    QAction* undoAction = new QAction(QIcon::fromTheme("edit-undo", QIcon(":/icons/undo.png")), tr("Undo"), this);
    QAction* redoAction = new QAction(QIcon::fromTheme("edit-redo", QIcon(":/icons/redo.png")), tr("Redo"), this);
    QAction* findAction = new QAction(QIcon::fromTheme("edit-find", QIcon(":/icons/find.png")), tr("Find"), this);

    // Add tooltips with keyboard shortcuts
    undoAction->setToolTip(tr("Undo (Ctrl+Z)"));
    redoAction->setToolTip(tr("Redo (Ctrl+Shift+Z)"));
    findAction->setToolTip(tr("Find (Ctrl+F)"));

    // Add actions to toolbar
    editorToolbar->addAction(undoAction);
    editorToolbar->addAction(redoAction);
    editorToolbar->addSeparator();
    editorToolbar->addAction(findAction);

    // Create code editor with IDE styling
    m_codeEditor = new QTextEdit(this);
    m_codeEditor->setLineWrapMode(QTextEdit::NoWrap);
    m_codeEditor->setStyleSheet(StyleKit::getCodeEditorStyle());

    // Set monospace font
    QFont font("JetBrains Mono", 11);  // Preferred coding font
    font.setStyleHint(QFont::Monospace);
    m_codeEditor->setFont(font);

    // Set tab width to 4 spaces
    QFontMetrics metrics(font);
    m_codeEditor->setTabStopDistance(4 * metrics.horizontalAdvance(' '));

    // Create the search widget (initially hidden)
    setupSearchWidget();

    // Status label with IDE styling
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setFrameStyle(QFrame::NoFrame);
    m_statusLabel->setMinimumHeight(24);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #F5F5F5;"  // Very light gray
        "  color: #333333;"             // Dark text
        "  border-top: 1px solid #BDBDBD;"  // Light gray top border
        "  padding: 4px 8px;"
        "}"
    );

    // Add to layout
    layout->addWidget(editorToolbar);
    layout->addWidget(m_findWidget);  // Initially hidden
    layout->addWidget(m_codeEditor, 1);
    layout->addWidget(m_statusLabel);

    // Connect signals for code editor
    connect(m_codeEditor, &QTextEdit::textChanged, this, &CodeEditorView::onTextChanged);

    // Connect toolbar actions
    connect(undoAction, &QAction::triggered, this, &CodeEditorView::undo);
    connect(redoAction, &QAction::triggered, this, &CodeEditorView::redo);
    connect(findAction, &QAction::triggered, this, &CodeEditorView::showFindWidget);

    // Set up keyboard shortcuts
    new QShortcut(QKeySequence::Undo, this, SLOT(undo()));
    new QShortcut(QKeySequence::Redo, this, SLOT(redo()));
    new QShortcut(QKeySequence::Find, this, SLOT(showFindWidget()));
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hideFindWidget()));
    new QShortcut(QKeySequence::FindNext, this, SLOT(findNext()));
    new QShortcut(QKeySequence::FindPrevious, this, SLOT(findPrevious()));
}

void CodeEditorView::setupSearchWidget()
{
    // Create search widget
    m_findWidget = new QWidget(this);
    m_findWidget->setObjectName("searchWidget");
    m_findWidget->setStyleSheet(StyleKit::getSearchWidgetStyle());
    m_findWidget->setVisible(false);  // Initially hidden

    QHBoxLayout* searchLayout = new QHBoxLayout(m_findWidget);
    searchLayout->setContentsMargins(6, 6, 6, 6);
    searchLayout->setSpacing(4);

    // Search field
    m_searchField = new QLineEdit(m_findWidget);
    m_searchField->setObjectName("searchField");
    m_searchField->setPlaceholderText(tr("Search"));

    // Case sensitivity checkbox
    m_caseSensitiveCheckBox = new QCheckBox(tr("Match case"), m_findWidget);

    // Previous/Next buttons
    QToolButton* prevButton = new QToolButton(m_findWidget);
    prevButton->setIcon(QIcon::fromTheme("go-up", QIcon(":/icons/up.png")));
    prevButton->setToolTip(tr("Previous match (Shift+F3)"));

    QToolButton* nextButton = new QToolButton(m_findWidget);
    nextButton->setIcon(QIcon::fromTheme("go-down", QIcon(":/icons/down.png")));
    nextButton->setToolTip(tr("Next match (F3)"));

    // Close button
    QPushButton* closeButton = new QPushButton(tr("×"), m_findWidget);
    closeButton->setObjectName("closeSearchButton");
    closeButton->setToolTip(tr("Close (Esc)"));
    closeButton->setFixedSize(16, 16);

    // Add widgets to layout
    searchLayout->addWidget(m_searchField);
    searchLayout->addWidget(m_caseSensitiveCheckBox);
    searchLayout->addWidget(prevButton);
    searchLayout->addWidget(nextButton);
    searchLayout->addWidget(closeButton);

    // Connect signals
    connect(m_searchField, &QLineEdit::textChanged, this, &CodeEditorView::searchTextChanged);
    connect(prevButton, &QToolButton::clicked, this, &CodeEditorView::findPrevious);
    connect(nextButton, &QToolButton::clicked, this, &CodeEditorView::findNext);
    connect(closeButton, &QPushButton::clicked, this, &CodeEditorView::hideFindWidget);
    connect(m_caseSensitiveCheckBox, &QCheckBox::toggled, this, &CodeEditorView::searchTextChanged);
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

void CodeEditorView::undo()
{
    if (m_codeEditor->document()->isUndoAvailable()) {
        m_codeEditor->undo();
    }
}

void CodeEditorView::redo()
{
    if (m_codeEditor->document()->isRedoAvailable()) {
        m_codeEditor->redo();
    }
}

void CodeEditorView::showFindWidget()
{
    m_findWidget->setVisible(true);
    m_searchField->setFocus();

    // If text is selected, use it as the search text
    QTextCursor cursor = m_codeEditor->textCursor();
    if (cursor.hasSelection()) {
        m_searchField->setText(cursor.selectedText());
        m_searchField->selectAll();
    }
}

void CodeEditorView::hideFindWidget()
{
    m_findWidget->setVisible(false);
    m_codeEditor->setFocus();
}

void CodeEditorView::searchTextChanged()
{
    // Reset the search when text changes
    m_lastSearchPos = -1;

    // Immediately search for the first occurrence
    if (!m_searchField->text().isEmpty()) {
        findNext();
    }
}

void CodeEditorView::findNext()
{
    if (!m_searchField || m_searchField->text().isEmpty()) {
        return;
    }

    QString searchText = m_searchField->text();
    QTextDocument::FindFlags flags;

    if (m_caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    QTextCursor cursor = m_codeEditor->textCursor();

    // Start from current position
    cursor = m_codeEditor->document()->find(searchText, cursor, flags);

    if (cursor.isNull()) {
        // Not found from current position, wrap around
        cursor = m_codeEditor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor = m_codeEditor->document()->find(searchText, cursor, flags);

        if (cursor.isNull()) {
            // Not found at all
            setStatusMessage(tr("No matches found for '%1'").arg(searchText), true);
            return;
        } else {
            setStatusMessage(tr("Search wrapped to beginning"));
        }
    }

    // Select the found text
    m_codeEditor->setTextCursor(cursor);

    // Update status
    int count = countOccurrences(searchText);
    setStatusMessage(tr("Match %1 of %2").arg(getCurrentMatchIndex(searchText) + 1).arg(count));
}

void CodeEditorView::findPrevious()
{
    if (!m_searchField || m_searchField->text().isEmpty()) {
        return;
    }

    QString searchText = m_searchField->text();
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;

    if (m_caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    QTextCursor cursor = m_codeEditor->textCursor();

    // Start from current position searching backward
    cursor = m_codeEditor->document()->find(searchText, cursor, flags);

    if (cursor.isNull()) {
        // Not found from current position, wrap around to end
        cursor = m_codeEditor->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor = m_codeEditor->document()->find(searchText, cursor, flags);

        if (cursor.isNull()) {
            // Not found at all
            setStatusMessage(tr("No matches found for '%1'").arg(searchText), true);
            return;
        } else {
            setStatusMessage(tr("Search wrapped to end"));
        }
    }

    // Select the found text
    m_codeEditor->setTextCursor(cursor);

    // Update status
    int count = countOccurrences(searchText);
    setStatusMessage(tr("Match %1 of %2").arg(getCurrentMatchIndex(searchText) + 1).arg(count));
}

int CodeEditorView::countOccurrences(const QString& text)
{
    if (text.isEmpty()) {
        return 0;
    }

    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    int count = 0;
    QTextCursor cursor(m_codeEditor->document());
    cursor.movePosition(QTextCursor::Start);

    while (!(cursor = m_codeEditor->document()->find(text, cursor, flags)).isNull()) {
        count++;
    }

    return count;
}

int CodeEditorView::getCurrentMatchIndex(const QString& text)
{
    if (text.isEmpty()) {
        return -1;
    }

    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    int index = 0;
    QTextCursor cursor(m_codeEditor->document());
    cursor.movePosition(QTextCursor::Start);

    QTextCursor currentCursor = m_codeEditor->textCursor();
    int currentPos = currentCursor.position();

    while (!(cursor = m_codeEditor->document()->find(text, cursor, flags)).isNull()) {
        if (cursor.position() >= currentPos) {
            return index;
        }
        index++;
    }

    return -1;
}

void CodeEditorView::setStatusMessage(const QString& message, bool isError)
{
    m_statusLabel->setText(message);

    if (isError) {
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #FFF0F0;"  // Very light red
            "  color: #C00000;"             // Dark red
            "  border-top: 1px solid #FFCCCC;"  // Light red top border
            "  padding: 4px 8px;"
            "}"
        );
    } else {
        m_statusLabel->setStyleSheet(
            "QLabel {"
            "  background-color: #F5F5F5;"  // Very light gray
            "  color: #333333;"             // Dark text
            "  border-top: 1px solid #BDBDBD;"  // Light gray top border
            "  padding: 4px 8px;"
            "}"
        );
    }
}