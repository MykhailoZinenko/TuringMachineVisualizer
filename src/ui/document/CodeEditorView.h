#pragma once

#include "DocumentView.h"
#include <memory>

class CodeDocument;
class QTextEdit;
class QPushButton;
class QLabel;

/**
 * View for editing Turing machine code
 */
class CodeEditorView : public DocumentView
{
    Q_OBJECT

public:
    CodeEditorView(CodeDocument* document, QWidget* parent = nullptr);
    ~CodeEditorView() override;

    // Update view from document
    void updateFromDocument() override;

    private slots:
        void onTextChanged();
    void applyChanges();
    void resetChanges();
    void createNewTape();

private:
    CodeDocument* m_codeDocument;

    // UI components
    QTextEdit* m_codeEditor;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    QPushButton* m_newTapeButton;
    QLabel* m_statusLabel;

    bool m_ignoreTextChanges;

    void setupUI();
    void setStatusMessage(const QString& message, bool isError = false);
};