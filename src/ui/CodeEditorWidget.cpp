#include "CodeEditorWidget.h"

#include <sstream>

// Qt includes
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QMessageBox>
#include <QDebug>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

// Project includes
#include "../model/TuringMachine.h"
#include "../model/State.h"
#include "../model/Transition.h"

// Syntax highlighter for the code editor
class TuringMachineSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    TuringMachineSyntaxHighlighter(QTextDocument* parent)
        : QSyntaxHighlighter(parent)
    {
        // Function notation format
        QTextCharFormat functionFormat;
        functionFormat.setForeground(Qt::darkBlue);
        functionFormat.setFontWeight(QFont::Bold);
        QRegularExpression functionRegex(R"(^(?:f)?\s*\()");
        highlightingRules.append({functionRegex, functionFormat});

        // State format
        QTextCharFormat stateFormat;
        stateFormat.setForeground(Qt::darkGreen);
        QRegularExpression stateRegex(R"(q[0-9a-zA-Z_]+)");
        highlightingRules.append({stateRegex, stateFormat});

        // Symbol format
        QTextCharFormat symbolFormat;
        symbolFormat.setForeground(Qt::red);
        QRegularExpression symbolRegex(R"(Blank|_|[0-9a-zA-Z])");
        highlightingRules.append({symbolRegex, symbolFormat});

        // Direction format
        QTextCharFormat directionFormat;
        directionFormat.setForeground(Qt::darkMagenta);
        QRegularExpression directionRegex(R"(R|L|N|0|S)");
        highlightingRules.append({directionRegex, directionFormat});

        // Comment format
        QTextCharFormat commentFormat;
        commentFormat.setForeground(Qt::gray);
        QRegularExpression commentRegex("//[^\n]*");
        highlightingRules.append({commentRegex, commentFormat});

        // Parentheses, commas, arrows format
        QTextCharFormat punctuationFormat;
        punctuationFormat.setForeground(Qt::black);
        punctuationFormat.setFontWeight(QFont::Bold);
        QRegularExpression punctuationRegex(R"([\(\),]|->|=)");
        highlightingRules.append({punctuationRegex, punctuationFormat});
    }

protected:
    void highlightBlock(const QString& text) override
    {
        for (const auto& rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
};

// CodeEditorWidget implementation
CodeEditorWidget::CodeEditorWidget(TuringMachine* machine, QWidget *parent)
    : QWidget(parent), m_machine(machine), m_ignoreTextChanges(false)
{
    setupUI();
    m_originalCode = "";

    // Only populate the editor if there's a machine
    if (machine) {
        updateFromModel();
    }
}

CodeEditorWidget::~CodeEditorWidget()
{
}

void CodeEditorWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Description label
    QLabel* descriptionLabel = new QLabel(tr("Edit the Turing machine using the code representation below:"), this);
    mainLayout->addWidget(descriptionLabel);

    // Code editor
    m_codeEditor = new QTextEdit(this);
    QFont font("Courier New", 10);
    m_codeEditor->setFont(font);
    mainLayout->addWidget(m_codeEditor);

    // Setup syntax highlighting
    new TuringMachineSyntaxHighlighter(m_codeEditor->document());

    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Status label
    m_statusLabel = new QLabel(this);
    buttonLayout->addWidget(m_statusLabel, 1);

    // Apply button
    m_applyButton = new QPushButton(tr("Apply"), this);
    buttonLayout->addWidget(m_applyButton);

    // Reset button
    m_resetButton = new QPushButton(tr("Reset"), this);
    buttonLayout->addWidget(m_resetButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_applyButton, &QPushButton::clicked, this, &CodeEditorWidget::applyCode);
    connect(m_resetButton, &QPushButton::clicked, this, &CodeEditorWidget::resetCode);
    connect(m_codeEditor, &QTextEdit::textChanged, this, &CodeEditorWidget::onTextChanged);

    // Initial state
    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);
    setStatus(tr("Ready"));
}

void CodeEditorWidget::setMachine(TuringMachine* machine)
{
    qDebug() << "CodeEditorWidget::setMachine called with" << (machine ? "valid" : "null") << "machine";
    if (machine) {
        qDebug() << "Machine has" << machine->getAllTransitions().size() << "transitions";
    }

    m_machine = machine;

    if (m_machine) {
        qDebug() << "After setting m_machine, it has" << m_machine->getAllTransitions().size() << "transitions";
    }
}

std::string CodeEditorWidget::getCurrentCode() const
{
    return m_codeEditor->toPlainText().toStdString();
}

// Improved updateFromModel method for CodeEditorWidget.cpp
void CodeEditorWidget::updateFromModel()
{
    qDebug() << "CodeEditorWidget::updateFromModel called";

    if (!m_machine) {
        qDebug() << "m_machine is null in updateFromModel";
        m_codeEditor->clear();
        m_applyButton->setEnabled(false);
        m_resetButton->setEnabled(false);
        return;
    }

    // Check if the machine has transitions
    auto transitions = m_machine->getAllTransitions();
    qDebug() << "Machine has" << transitions.size() << "transitions in updateFromModel";

    m_ignoreTextChanges = true;
    try {
        // First try to use original code from the machine if available
        std::string originalCode = m_machine->getOriginalCode();

        if (!originalCode.empty()) {
            // Use the original code if available
            qDebug() << "Using original code from machine, size:" << originalCode.size();
            m_codeEditor->setPlainText(QString::fromStdString(originalCode));
            m_originalCode = originalCode;
        } else {
            // Fall back to generating code from transitions
            qDebug() << "No original code, generating from transitions";
            std::string generatedCode = generateCodeFromModel();

            if (generatedCode.empty() && !transitions.empty()) {
                // If code generation fails but we have transitions, try a fallback approach
                qDebug() << "Generated code is empty despite having transitions. Using fallback.";
                generatedCode = "// " + m_machine->getName() + "\n\n";
                for (const auto& transition : transitions) {
                    if (transition) {
                        try {
                            generatedCode += transition->toFunctionNotation() + "\n";
                        } catch (const std::exception& e) {
                            qWarning() << "Error generating function notation for transition:" << e.what();
                            // Add a simple representation as fallback
                            generatedCode += "// Failed to format: " + transition->getFromState() + " -> " +
                                    transition->getToState() + " on " + transition->getReadSymbol() + "\n";
                        }
                    }
                }
            }

            m_codeEditor->setPlainText(QString::fromStdString(generatedCode));
            m_originalCode = generatedCode;

            // Also update the machine's original code with our generated version
            m_machine->setOriginalCode(generatedCode);
        }
    } catch (const std::exception& e) {
        qCritical() << "Exception in updateFromModel:" << e.what();
        m_codeEditor->setPlainText(tr("// Error generating code: %1").arg(e.what()));
    } catch (...) {
        qCritical() << "Unknown exception in updateFromModel";
        m_codeEditor->setPlainText(tr("// Unknown error generating code"));
    }
    m_ignoreTextChanges = false;

    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);
    setStatus(tr("Ready"));
}

bool CodeEditorWidget::updateMachineFromCode()
{
    std::string code = getCurrentCode();
    return parseCodeAndUpdateModel(code);
}

void CodeEditorWidget::appendTransition(const Transition* transition)
{
    if (!transition) return;

    // Get current code
    std::string code = getCurrentCode();

    // Add a newline if needed
    if (!code.empty() && code.back() != '\n') {
        code += "\n";
    }

    // Add the transition in functional notation
    code += transition->toFunctionNotation() + "\n";

    // Update the editor with the new code
    m_ignoreTextChanges = true;
    m_codeEditor->setPlainText(QString::fromStdString(code));
    m_ignoreTextChanges = false;

    // Save as the new original code
    m_originalCode = code;

    // Apply changes to the model
    parseCodeAndUpdateModel(code);

    setStatus(tr("Added new transition"));
}

bool CodeEditorWidget::applyCodeChanges() {
    if (m_applyButton) {
        m_applyButton->click();
        return true;
    }
    return false;
}

std::string CodeEditorWidget::generateCodeFromModel()
{
    if (!m_machine) return "";

    qDebug() << "Generating code from model with" << m_machine->getAllTransitions().size() << "transitions";
    std::string code;

    // Machine name as comment header
    code += "// " + m_machine->getName() + "\n\n";

    try {
        // Generate transitions using function notation
        auto transitions = m_machine->getAllTransitions();
        if (transitions.empty()) {
            qDebug() << "No transitions found in the machine";
            return code;
        }

        for (const auto& transition : transitions) {
            if (!transition) {
                qWarning() << "Null transition pointer encountered";
                continue;
            }

            try {
                std::string transitionCode = transition->toFunctionNotation();
                code += transitionCode + "\n";
                qDebug() << "Added transition:" << QString::fromStdString(transitionCode);
            } catch (const std::exception& e) {
                qWarning() << "Exception while formatting transition:" << e.what();
                // Add a fallback representation
                code += "// Error formatting: " + transition->getFromState() + " on " +
                        transition->getReadSymbol() + "\n";
            }
        }
    } catch (const std::exception& e) {
        // Log any exceptions and return a safe error message
        qCritical() << "Exception in generateCodeFromModel:" << e.what();
        code += "// Error generating code: " + std::string(e.what()) + "\n";
    } catch (...) {
        // Catch any other errors
        qCritical() << "Unknown exception in generateCodeFromModel";
        code += "// Unknown error generating code\n";
    }

    qDebug() << "Generated code size:" << code.size() << "bytes";
    return code;
}

bool CodeEditorWidget::parseCodeAndUpdateModel(const std::string& code)
{
    try {
        // Create a temporary machine to build
        auto newMachine = std::make_unique<TuringMachine>(m_machine->getName());

        // Track states to add them correctly
        std::set<std::string> stateIds;
        std::string startState;
        std::map<std::string, StateType> stateTypes;

        // Process each line
        std::istringstream stream(code);
        std::string line;

        std::vector<Transition> transitions;

        while (std::getline(stream, line)) {
            // Skip empty lines and comments
            if (line.empty() || line.substr(0, 2) == "//") continue;

            // Try to parse as transition in function notation
            try {
                Transition transition = Transition::fromFunctionNotation(line);
                if (transition.isValid()) {
                    transitions.push_back(transition);

                    // Add states to our set
                    stateIds.insert(transition.getFromState());
                    stateIds.insert(transition.getToState());
                }
            } catch (const std::exception& e) {
                // Not a valid transition, just ignore and continue
                setStatus(tr("Warning: Could not parse line: %1").arg(line.c_str()), true);
            }
        }

        // No transitions found
        if (transitions.empty()) {
            setStatus(tr("No valid transitions found in code"), true);
            return false;
        }

        // Determine heuristically which is the start state (first from state encountered)
        if (!transitions.empty() && startState.empty()) {
            startState = transitions.front().getFromState();
        }

        // Add all states to the machine
        for (const auto& stateId : stateIds) {
            StateType type = StateType::NORMAL;
            if (stateId == startState) {
                type = StateType::START;
            } else if (stateTypes.count(stateId)) {
                type = stateTypes[stateId];
            }

            newMachine->addState(stateId, "", type);
        }

        // Set the start state
        if (!startState.empty()) {
            newMachine->setStartState(startState);
        }

        // Add all transitions
        for (const auto& transition : transitions) {
            newMachine->addTransition(
                transition.getFromState(),
                transition.getReadSymbol(),
                transition.getToState(),
                transition.getWriteSymbol(),
                transition.getDirection()
            );
        }

        // If we get here, parsing was successful

        // Copy machine properties
        m_machine->setName(newMachine->getName());
        m_machine->setType(newMachine->getType());

        // Clear existing states and transitions
        auto existingStates = m_machine->getAllStates();
        for (auto state : existingStates) {
            m_machine->removeState(state->getId());
        }

        // Copy states
        auto newStates = newMachine->getAllStates();
        for (auto state : newStates) {
            m_machine->addState(state->getId(), state->getName(), state->getType());
            State* existingState = m_machine->getState(state->getId());
            if (existingState) {
                existingState->setPosition(state->getPosition());
            }
        }

        // Copy start state
        if (!startState.empty()) {
            m_machine->setStartState(startState);
        }

        // Copy transitions
        auto newTransitions = newMachine->getAllTransitions();
        for (auto transition : newTransitions) {
            m_machine->addTransition(
                transition->getFromState(),
                transition->getReadSymbol(),
                transition->getToState(),
                transition->getWriteSymbol(),
                transition->getDirection()
            );
        }

        return true;
    } catch (const std::exception& e) {
        setStatus(tr("Error parsing code: %1").arg(e.what()), true);
        return false;
    }
}

void CodeEditorWidget::applyCode()
{
    std::string code = getCurrentCode();
    m_originalCode = code; // Save the code to preserve formatting

    if (parseCodeAndUpdateModel(code)) {
        // Also save the code to the machine for persistence
        if (m_machine) {
            m_machine->setOriginalCode(code);
            qDebug() << "Saved code to machine, size:" << code.size();
        }

        setStatus(tr("Code applied successfully!"));
        m_applyButton->setEnabled(false);
        m_resetButton->setEnabled(false);
        emit codeChanged();
    }
}

void CodeEditorWidget::resetCode()
{
    if (!m_originalCode.empty()) {
        // Restore to the original saved code
        m_ignoreTextChanges = true;
        m_codeEditor->setPlainText(QString::fromStdString(m_originalCode));
        m_ignoreTextChanges = false;

        m_applyButton->setEnabled(false);
        m_resetButton->setEnabled(false);
        setStatus(tr("Reset to original code"));
    } else {
        // If no original code, regenerate from model
        updateFromModel();
        setStatus(tr("Reset to model-generated code"));
    }
}

void CodeEditorWidget::onTextChanged()
{
    if (m_ignoreTextChanges) return;
    
    m_applyButton->setEnabled(true);
    m_resetButton->setEnabled(true);
    setStatus(tr("Modified - click Apply to update the machine"));
}

void CodeEditorWidget::setStatus(const QString& message, bool isError)
{
    m_statusLabel->setText(message);
    
    QPalette pal = m_statusLabel->palette();
    pal.setColor(QPalette::WindowText, isError ? Qt::red : Qt::black);
    m_statusLabel->setPalette(pal);
}