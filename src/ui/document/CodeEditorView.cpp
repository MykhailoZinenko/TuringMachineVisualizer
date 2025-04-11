#include "CodeEditorView.h"
#include "../../document/CodeDocument.h"
#include "../../project/Project.h"
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QInputDialog>
#include <QMessageBox>

#include "document/TapeDocument.h"

CodeEditorView::CodeEditorView(CodeDocument* document, QWidget* parent)
    : DocumentView(document, parent),
      m_codeDocument(document),
      m_ignoreTextChanges(false)
{
    setupUI();
    updateFromDocument();
}

CodeEditorView::~CodeEditorView()
{
}

void CodeEditorView::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Header label
    QLabel* headerLabel = new QLabel(tr("Edit Turing Machine Code"), this);
    QFont headerFont = headerLabel->font();
    headerFont.setBold(true);
    headerFont.setPointSize(headerFont.pointSize() + 2);
    headerLabel->setFont(headerFont);
    mainLayout->addWidget(headerLabel);

    // Code editor
    m_codeEditor = new QTextEdit(this);
    QFont codeFont("Courier New", 10);
    m_codeEditor->setFont(codeFont);
    mainLayout->addWidget(m_codeEditor);

    // Bottom controls
    QHBoxLayout* bottomLayout = new QHBoxLayout();

    // Status label
    m_statusLabel = new QLabel(tr("Ready"), this);
    bottomLayout->addWidget(m_statusLabel, 1);

    // New tape button
    m_newTapeButton = new QPushButton(tr("New Tape"), this);
    connect(m_newTapeButton, &QPushButton::clicked, this, &CodeEditorView::createNewTape);
    bottomLayout->addWidget(m_newTapeButton);

    // Reset button
    m_resetButton = new QPushButton(tr("Reset"), this);
    connect(m_resetButton, &QPushButton::clicked, this, &CodeEditorView::resetChanges);
    bottomLayout->addWidget(m_resetButton);

    // Apply button
    m_applyButton = new QPushButton(tr("Apply"), this);
    connect(m_applyButton, &QPushButton::clicked, this, &CodeEditorView::applyChanges);
    bottomLayout->addWidget(m_applyButton);

    mainLayout->addLayout(bottomLayout);

    // Connect signals
    connect(m_codeEditor, &QTextEdit::textChanged, this, &CodeEditorView::onTextChanged);

    // Initial state
    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);
}

void CodeEditorView::updateFromDocument()
{
    if (!m_codeDocument) return;

    m_ignoreTextChanges = true;
    m_codeEditor->setPlainText(QString::fromStdString(m_codeDocument->getCode()));
    m_ignoreTextChanges = false;

    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);

    setStatusMessage(tr("Code loaded from document"));
}

void CodeEditorView::onTextChanged()
{
    if (m_ignoreTextChanges) return;

    m_applyButton->setEnabled(true);
    m_resetButton->setEnabled(true);
    setStatusMessage(tr("Modified - click Apply to update the machine"));
}

void CodeEditorView::applyChanges()
{
    if (!m_codeDocument) return;

    std::string newCode = m_codeEditor->toPlainText().toStdString();
    m_codeDocument->setCode(newCode);

    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);

    setStatusMessage(tr("Changes applied successfully"));
    emit viewModified();
}

void CodeEditorView::resetChanges()
{
    updateFromDocument();
}

void CodeEditorView::createNewTape()
{
    // First make sure any code changes are applied
    if (m_applyButton->isEnabled()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Apply Changes"),
            tr("You have unsaved code changes. Apply them before creating a new tape?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
        );

        if (result == QMessageBox::Yes) {
            applyChanges();
        } else if (result == QMessageBox::Cancel) {
            return;
        }
    }

    if (!m_codeDocument || !m_codeDocument->getProject()) {
        setStatusMessage(tr("No project available"), true);
        return;
    }

    // Get tape name
    bool ok;
    QString tapeName = QInputDialog::getText(
        this,
        tr("New Tape"),
        tr("Enter a name for the new tape:"),
        QLineEdit::Normal,
        tr("Tape for %1").arg(QString::fromStdString(m_codeDocument->getName())),
        &ok
    );

    if (!ok || tapeName.isEmpty()) return;

    // Get initial content
    QString initialContent = QInputDialog::getText(
        this,
        tr("Initial Content"),
        tr("Enter initial tape content:"),
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok) return;

    // Create the tape document
    TapeDocument* tapeDoc = m_codeDocument->getProject()->createTape(tapeName.toStdString());
    
    if (tapeDoc) {
        tapeDoc->setInitialContent(initialContent.toStdString());
        setStatusMessage(tr("Created new tape: %1").arg(tapeName));
    } else {
        setStatusMessage(tr("Failed to create new tape"), true);
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