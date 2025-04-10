#pragma once

#include <QWidget>
#include <QColor>
#include <string>

// Forward declarations
class QStackedWidget;
class QVBoxLayout;
class QFormLayout;
class QLineEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QGroupBox;
class QSpinBox;

class TuringMachine;
class State;
class Transition;

enum class EditorMode {
    NoSelection,
    MachineSelected,
    StateSelected,
    TransitionSelected
};

class PropertiesEditorWidget : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit PropertiesEditorWidget(TuringMachine* machine, QWidget *parent = nullptr);

    // Selection methods
    void setMachine(TuringMachine* machine);
    void clearSelection();
    void selectState(const std::string& stateId);
    void selectTransition(const std::string& fromState, const std::string& readSymbol); // Changed to string

signals:
    void machinePropertiesChanged();
    void statePropertiesChanged(const std::string& stateId);
    void transitionPropertiesChanged(const std::string& fromState, const std::string& readSymbol); // Changed to string

private slots:
    // Update slots
    void updateMachineProperties();
    void updateStateProperties();
    void updateTransitionProperties();

    // UI interaction slots
    void pickStateColor();
    void applyChanges();
    void resetChanges();

private:
    // Data
    TuringMachine* m_machine;
    State* m_selectedState;
    Transition* m_selectedTransition;
    EditorMode m_currentMode;
    QColor m_stateColor;
    std::string m_selectedFromState;   // Store the fromState for the selected transition
    std::string m_selectedReadSymbol;  // Store the readSymbol for the selected transition

    // UI components - Main
    QStackedWidget* m_stackedWidget;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;

    // UI components - Machine editor
    QWidget* m_machineEditorWidget;
    QLineEdit* m_machineNameEdit;
    QComboBox* m_machineTypeCombo;

    // UI components - State editor
    QWidget* m_stateEditorWidget;
    QLabel* m_stateIdLabel;
    QLineEdit* m_stateNameEdit;
    QComboBox* m_stateTypeCombo;
    QPushButton* m_stateColorButton;
    QSpinBox* m_stateXSpin;
    QSpinBox* m_stateYSpin;

    // UI components - Transition editor
    QWidget* m_transitionEditorWidget;
    QLabel* m_transFromLabel;
    QLabel* m_transReadLabel;
    QComboBox* m_transToStateCombo;
    QLineEdit* m_transWriteEdit;
    QComboBox* m_transDirectionCombo;

    // UI components - No selection view
    QWidget* m_noSelectionWidget;

    // Setup methods
    void setupUI();
    void setupMachineEditor();
    void setupStateEditor();
    void setupTransitionEditor();
    void setupNoSelectionView();

    // UI update methods
    void updateUIForMachine();
    void updateUIForState();
    void updateUIForTransition();
    void updateUIForNoSelection();

    // Helper methods
    void populateStateComboBox(QComboBox* comboBox, const std::string& selectedStateId = "");
};