#ifndef PROPERTIESEDITORWIDGET_H
#define PROPERTIESEDITORWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QColorDialog>
#include <QSpinBox>
#include "../model/TuringMachine.h"
#include "../model/State.h"
#include "../model/Transition.h"

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
    explicit PropertiesEditorWidget(TuringMachine* machine, QWidget *parent = nullptr);
    
    void setMachine(TuringMachine* machine);
    void clearSelection();
    void selectState(const std::string& stateId);
    void selectTransition(const std::string& fromState, char readSymbol);

signals:
    void machinePropertiesChanged();
    void statePropertiesChanged(const std::string& stateId);
    void transitionPropertiesChanged(const std::string& fromState, char readSymbol);

private slots:
    void updateMachineProperties();
    void updateStateProperties();
    void updateTransitionProperties();
    void pickStateColor();
    void applyChanges();
    void resetChanges();

private:
    TuringMachine* m_machine;
    State* m_selectedState;
    Transition* m_selectedTransition;
    EditorMode m_currentMode;
    
    QStackedWidget* m_stackedWidget;
    
    // Machine editor widgets
    QWidget* m_machineEditorWidget;
    QLineEdit* m_machineNameEdit;
    QComboBox* m_machineTypeCombo;
    
    // State editor widgets
    QWidget* m_stateEditorWidget;
    QLabel* m_stateIdLabel;
    QLineEdit* m_stateNameEdit;
    QComboBox* m_stateTypeCombo;
    QPushButton* m_stateColorButton;
    QColor m_stateColor;
    QSpinBox* m_stateXSpin;
    QSpinBox* m_stateYSpin;
    
    // Transition editor widgets
    QWidget* m_transitionEditorWidget;
    QLabel* m_transFromLabel;
    QLabel* m_transReadLabel;
    QComboBox* m_transToStateCombo;
    QLineEdit* m_transWriteEdit;
    QComboBox* m_transDirectionCombo;
    
    // No selection widget
    QWidget* m_noSelectionWidget;
    
    // Common buttons
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    
    void setupUI();
    void setupMachineEditor();
    void setupStateEditor();
    void setupTransitionEditor();
    void setupNoSelectionView();
    
    void updateUIForMachine();
    void updateUIForState();
    void updateUIForTransition();
    void updateUIForNoSelection();
    
    void populateStateComboBox(QComboBox* comboBox, const std::string& selectedStateId = "");
};

#endif // PROPERTIESEDITORWIDGET_H