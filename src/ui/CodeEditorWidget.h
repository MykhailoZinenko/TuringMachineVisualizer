#pragma once

#include <QWidget>
#include <string>

// Forward declarations
class QTextEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class TuringMachine;
class Transition;

class CodeEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CodeEditorWidget(TuringMachine* machine, QWidget *parent = nullptr);
    ~CodeEditorWidget();

    // Update the code editor from the current machine model - should only be called on initial load
    void updateFromModel();

    // Set the machine to edit
    void setMachine(TuringMachine* machine);

    // Used to provide the current code to other components
    std::string getCurrentCode() const;

    // Update only the machine (not the editor) from the current code
    bool updateMachineFromCode();

    void appendTransition(const Transition* transition);

    signals:
        // Signal emitted when the code has been changed and the model updated
        void codeChanged();

    private slots:
        // Apply the code changes to the model
        void applyCode();

    // Reset to the current model state
    void resetCode();

    // Text changed slot to track modifications
    void onTextChanged();

private:
    TuringMachine* m_machine;
    QTextEdit* m_codeEditor;
    QLabel* m_statusLabel;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;

    bool m_ignoreTextChanges;  // Flag to prevent recursive updates
    std::string m_originalCode; // Store original code to preserve format

    // Setup UI components
    void setupUI();

    // Generate code from the machine model
    std::string generateCodeFromModel();

    // Parse code and update the machine model
    bool parseCodeAndUpdateModel(const std::string& code);

    // Update the status label
    void setStatus(const QString& message, bool isError = false);
};