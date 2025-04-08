#include "PropertiesEditorWidget.h"
#include <QMessageBox>

PropertiesEditorWidget::PropertiesEditorWidget(TuringMachine* machine, QWidget *parent)
    : QWidget(parent), m_machine(machine), m_selectedState(nullptr), 
      m_selectedTransition(nullptr), m_currentMode(EditorMode::NoSelection),
      m_stateColor(Qt::white)
{
    setupUI();
    updateUIForNoSelection();
}

void PropertiesEditorWidget::setMachine(TuringMachine* machine)
{
    m_machine = machine;
    m_selectedState = nullptr;
    m_selectedTransition = nullptr;
    m_currentMode = EditorMode::MachineSelected;
    updateUIForMachine();
}

void PropertiesEditorWidget::clearSelection()
{
    m_selectedState = nullptr;
    m_selectedTransition = nullptr;
    m_currentMode = EditorMode::NoSelection;
    updateUIForNoSelection();
}

void PropertiesEditorWidget::selectState(const std::string& stateId)
{
    if (!m_machine) return;
    
    m_selectedState = m_machine->getState(stateId);
    if (m_selectedState) {
        m_selectedTransition = nullptr;
        m_currentMode = EditorMode::StateSelected;
        updateUIForState();
    }
}

void PropertiesEditorWidget::selectTransition(const std::string& fromState, char readSymbol)
{
    if (!m_machine) return;
    
    m_selectedTransition = m_machine->getTransition(fromState, readSymbol);
    if (m_selectedTransition) {
        m_selectedState = nullptr;
        m_currentMode = EditorMode::TransitionSelected;
        updateUIForTransition();
    }
}

void PropertiesEditorWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create stacked widget to hold different editors
    m_stackedWidget = new QStackedWidget(this);
    
    // Setup individual editors
    setupMachineEditor();
    setupStateEditor();
    setupTransitionEditor();
    setupNoSelectionView();
    
    // Add editors to stacked widget
    m_stackedWidget->addWidget(m_noSelectionWidget);     // index 0
    m_stackedWidget->addWidget(m_machineEditorWidget);   // index 1
    m_stackedWidget->addWidget(m_stateEditorWidget);     // index 2
    m_stackedWidget->addWidget(m_transitionEditorWidget); // index 3
    
    // Create buttons for applying/resetting changes
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_applyButton = new QPushButton(tr("Apply"), this);
    m_resetButton = new QPushButton(tr("Reset"), this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addWidget(m_applyButton);
    
    // Connect buttons
    connect(m_applyButton, &QPushButton::clicked, this, &PropertiesEditorWidget::applyChanges);
    connect(m_resetButton, &QPushButton::clicked, this, &PropertiesEditorWidget::resetChanges);
    
    // Add widgets to main layout
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->addLayout(buttonLayout);
    
    // Initial state
    m_stackedWidget->setCurrentIndex(0); // No selection view
    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);
}

void PropertiesEditorWidget::setupMachineEditor()
{
    m_machineEditorWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_machineEditorWidget);
    
    QGroupBox* groupBox = new QGroupBox(tr("Machine Properties"), m_machineEditorWidget);
    QFormLayout* formLayout = new QFormLayout(groupBox);
    
    // Machine name
    m_machineNameEdit = new QLineEdit(m_machineEditorWidget);
    formLayout->addRow(tr("Name:"), m_machineNameEdit);
    
    // Machine type
    m_machineTypeCombo = new QComboBox(m_machineEditorWidget);
    m_machineTypeCombo->addItem(tr("Deterministic"), static_cast<int>(MachineType::DETERMINISTIC));
    m_machineTypeCombo->addItem(tr("Non-Deterministic"), static_cast<int>(MachineType::NON_DETERMINISTIC));
    formLayout->addRow(tr("Type:"), m_machineTypeCombo);
    
    layout->addWidget(groupBox);
    layout->addStretch();
}

void PropertiesEditorWidget::setupStateEditor()
{
    m_stateEditorWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_stateEditorWidget);
    
    QGroupBox* groupBox = new QGroupBox(tr("State Properties"), m_stateEditorWidget);
    QFormLayout* formLayout = new QFormLayout(groupBox);
    
    // State ID (read-only)
    m_stateIdLabel = new QLabel(m_stateEditorWidget);
    formLayout->addRow(tr("ID:"), m_stateIdLabel);
    
    // State name
    m_stateNameEdit = new QLineEdit(m_stateEditorWidget);
    formLayout->addRow(tr("Display Name:"), m_stateNameEdit);
    
    // State type
    m_stateTypeCombo = new QComboBox(m_stateEditorWidget);
    m_stateTypeCombo->addItem(tr("Normal"), static_cast<int>(StateType::NORMAL));
    m_stateTypeCombo->addItem(tr("Start"), static_cast<int>(StateType::START));
    m_stateTypeCombo->addItem(tr("Accept"), static_cast<int>(StateType::ACCEPT));
    m_stateTypeCombo->addItem(tr("Reject"), static_cast<int>(StateType::REJECT));
    formLayout->addRow(tr("Type:"), m_stateTypeCombo);
    
    // State color
    QHBoxLayout* colorLayout = new QHBoxLayout();
    m_stateColorButton = new QPushButton(m_stateEditorWidget);
    m_stateColorButton->setFixedWidth(80);
    m_stateColorButton->setAutoFillBackground(true);
    colorLayout->addWidget(m_stateColorButton);
    colorLayout->addStretch();
    formLayout->addRow(tr("Color:"), colorLayout);
    
    // Connect color button
    connect(m_stateColorButton, &QPushButton::clicked, this, &PropertiesEditorWidget::pickStateColor);
    
    // State position
    QHBoxLayout* posLayout = new QHBoxLayout();
    m_stateXSpin = new QSpinBox(m_stateEditorWidget);
    m_stateXSpin->setRange(-1000, 1000);
    m_stateYSpin = new QSpinBox(m_stateEditorWidget);
    m_stateYSpin->setRange(-1000, 1000);
    posLayout->addWidget(m_stateXSpin);
    posLayout->addWidget(new QLabel("Y:"));
    posLayout->addWidget(m_stateYSpin);
    formLayout->addRow(tr("Position X:"), posLayout);
    
    layout->addWidget(groupBox);
    layout->addStretch();
}

void PropertiesEditorWidget::setupTransitionEditor()
{
    m_transitionEditorWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_transitionEditorWidget);
    
    QGroupBox* groupBox = new QGroupBox(tr("Transition Properties"), m_transitionEditorWidget);
    QFormLayout* formLayout = new QFormLayout(groupBox);
    
    // From state (read-only)
    m_transFromLabel = new QLabel(m_transitionEditorWidget);
    formLayout->addRow(tr("From State:"), m_transFromLabel);
    
    // Read symbol (read-only)
    m_transReadLabel = new QLabel(m_transitionEditorWidget);
    formLayout->addRow(tr("Read Symbol:"), m_transReadLabel);
    
    // To state
    m_transToStateCombo = new QComboBox(m_transitionEditorWidget);
    formLayout->addRow(tr("To State:"), m_transToStateCombo);
    
    // Write symbol
    m_transWriteEdit = new QLineEdit(m_transitionEditorWidget);
    m_transWriteEdit->setMaxLength(1);
    formLayout->addRow(tr("Write Symbol:"), m_transWriteEdit);
    
    // Direction
    m_transDirectionCombo = new QComboBox(m_transitionEditorWidget);
    m_transDirectionCombo->addItem(tr("Left"), static_cast<int>(Direction::LEFT));
    m_transDirectionCombo->addItem(tr("Right"), static_cast<int>(Direction::RIGHT));
    m_transDirectionCombo->addItem(tr("Stay"), static_cast<int>(Direction::STAY));
    formLayout->addRow(tr("Direction:"), m_transDirectionCombo);
    
    layout->addWidget(groupBox);
    layout->addStretch();
}

void PropertiesEditorWidget::setupNoSelectionView()
{
    m_noSelectionWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_noSelectionWidget);
    
    QLabel* label = new QLabel(tr("Select a machine, state, or transition to edit its properties."), m_noSelectionWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    
    layout->addStretch();
    layout->addWidget(label);
    layout->addStretch();
}

void PropertiesEditorWidget::updateUIForMachine()
{
    if (!m_machine) {
        updateUIForNoSelection();
        return;
    }
    
    // Update machine properties
    m_machineNameEdit->setText(QString::fromStdString(m_machine->getName()));
    int typeIndex = m_machineTypeCombo->findData(static_cast<int>(m_machine->getType()));
    if (typeIndex >= 0) {
        m_machineTypeCombo->setCurrentIndex(typeIndex);
    }
    
    // Switch to machine editor
    m_stackedWidget->setCurrentWidget(m_machineEditorWidget);
    
    // Enable buttons
    m_applyButton->setEnabled(true);
    m_resetButton->setEnabled(true);
}

void PropertiesEditorWidget::updateUIForState()
{
    if (!m_selectedState) {
        updateUIForNoSelection();
        return;
    }
    
    // Update state properties
    m_stateIdLabel->setText(QString::fromStdString(m_selectedState->getId()));
    m_stateNameEdit->setText(QString::fromStdString(m_selectedState->getName()));
    
    int typeIndex = m_stateTypeCombo->findData(static_cast<int>(m_selectedState->getType()));
    if (typeIndex >= 0) {
        m_stateTypeCombo->setCurrentIndex(typeIndex);
    }
    
    // Set color button background
    QPalette pal = m_stateColorButton->palette();
    pal.setColor(QPalette::Button, m_stateColor);
    m_stateColorButton->setPalette(pal);
    
    // Set position
    QPointF pos = m_selectedState->getPosition();
    m_stateXSpin->setValue(static_cast<int>(pos.x()));
    m_stateYSpin->setValue(static_cast<int>(pos.y()));
    
    // Switch to state editor
    m_stackedWidget->setCurrentWidget(m_stateEditorWidget);
    
    // Enable buttons
    m_applyButton->setEnabled(true);
    m_resetButton->setEnabled(true);
}

void PropertiesEditorWidget::updateUIForTransition()
{
    if (!m_selectedTransition) {
        updateUIForNoSelection();
        return;
    }
    
    // Update transition properties
    m_transFromLabel->setText(QString::fromStdString(m_selectedTransition->getFromState()));
    m_transReadLabel->setText(QString(m_selectedTransition->getReadSymbol()));
    
    // Populate to state combo
    populateStateComboBox(m_transToStateCombo, m_selectedTransition->getToState());
    
    // Set write symbol
    m_transWriteEdit->setText(QString(m_selectedTransition->getWriteSymbol()));
    
    // Set direction
    int dirIndex = m_transDirectionCombo->findData(static_cast<int>(m_selectedTransition->getDirection()));
    if (dirIndex >= 0) {
        m_transDirectionCombo->setCurrentIndex(dirIndex);
    }
    
    // Switch to transition editor
    m_stackedWidget->setCurrentWidget(m_transitionEditorWidget);
    
    // Enable buttons
    m_applyButton->setEnabled(true);
    m_resetButton->setEnabled(true);
}

void PropertiesEditorWidget::updateUIForNoSelection()
{
    // Switch to no selection view
    m_stackedWidget->setCurrentWidget(m_noSelectionWidget);
    
    // Disable buttons
    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);
}

void PropertiesEditorWidget::populateStateComboBox(QComboBox* comboBox, const std::string& selectedStateId)
{
    comboBox->clear();
    
    if (!m_machine) return;
    
    auto states = m_machine->getAllStates();
    for (auto state : states) {
        QString displayText = QString::fromStdString(state->getId());
        if (!state->getName().empty()) {
            displayText += " (" + QString::fromStdString(state->getName()) + ")";
        }
        comboBox->addItem(displayText, QString::fromStdString(state->getId()));
    }
    
    // Select the specified state if it exists
    if (!selectedStateId.empty()) {
        int index = comboBox->findData(QString::fromStdString(selectedStateId));
        if (index >= 0) {
            comboBox->setCurrentIndex(index);
        }
    }
}

void PropertiesEditorWidget::pickStateColor()
{
    QColor color = QColorDialog::getColor(m_stateColor, this, tr("Select State Color"));
    if (color.isValid()) {
        m_stateColor = color;
        
        // Update button color
        QPalette pal = m_stateColorButton->palette();
        pal.setColor(QPalette::Button, m_stateColor);
        m_stateColorButton->setPalette(pal);
        m_stateColorButton->update();
    }
}

void PropertiesEditorWidget::applyChanges()
{
    if (!m_machine) return;
    
    switch (m_currentMode) {
        case EditorMode::MachineSelected:
            updateMachineProperties();
            break;
            
        case EditorMode::StateSelected:
            updateStateProperties();
            break;
            
        case EditorMode::TransitionSelected:
            updateTransitionProperties();
            break;
            
        case EditorMode::NoSelection:
            // Nothing to do
            break;
    }
}

void PropertiesEditorWidget::resetChanges()
{
    // Reset UI based on current mode
    switch (m_currentMode) {
        case EditorMode::MachineSelected:
            updateUIForMachine();
            break;
            
        case EditorMode::StateSelected:
            updateUIForState();
            break;
            
        case EditorMode::TransitionSelected:
            updateUIForTransition();
            break;
            
        case EditorMode::NoSelection:
            updateUIForNoSelection();
            break;
    }
}

void PropertiesEditorWidget::updateMachineProperties()
{
    QString oldName = QString::fromStdString(m_machine->getName());
    MachineType oldType = m_machine->getType();
    
    // Update machine properties
    QString newName = m_machineNameEdit->text();
    MachineType newType = static_cast<MachineType>(m_machineTypeCombo->currentData().toInt());
    
    bool changed = false;
    
    if (newName != oldName) {
        m_machine->setName(newName.toStdString());
        changed = true;
    }
    
    if (newType != oldType) {
        m_machine->setType(newType);
        changed = true;
    }
    
    if (changed) {
        emit machinePropertiesChanged();
    }
}

void PropertiesEditorWidget::updateStateProperties()
{
    if (!m_selectedState) return;
    
    std::string stateId = m_selectedState->getId();
    std::string oldName = m_selectedState->getName();
    StateType oldType = m_selectedState->getType();
    QPointF oldPos = m_selectedState->getPosition();
    
    // Update state properties
    std::string newName = m_stateNameEdit->text().toStdString();
    StateType newType = static_cast<StateType>(m_stateTypeCombo->currentData().toInt());
    QPointF newPos(m_stateXSpin->value(), m_stateYSpin->value());
    
    bool changed = false;
    
    if (newName != oldName) {
        m_selectedState->setName(newName);
        changed = true;
    }
    
    if (newType != oldType) {
        m_selectedState->setType(newType);
        
        // If changed to START, update the machine's start state
        if (newType == StateType::START) {
            m_machine->setStartState(stateId);
        }
        
        changed = true;
    }
    
    if (newPos != oldPos) {
        m_selectedState->setPosition(newPos);
        changed = true;
    }
    
    // Note: Color changes would be saved here when implemented in the State class
    
    if (changed) {
        emit statePropertiesChanged(stateId);
    }
}

void PropertiesEditorWidget::updateTransitionProperties()
{
    if (!m_selectedTransition) return;
    
    std::string fromState = m_selectedTransition->getFromState();
    char readSymbol = m_selectedTransition->getReadSymbol();
    
    std::string oldToState = m_selectedTransition->getToState();
    char oldWriteSymbol = m_selectedTransition->getWriteSymbol();
    Direction oldDirection = m_selectedTransition->getDirection();
    
    // Get new values
    std::string newToState = m_transToStateCombo->currentData().toString().toStdString();
    
    QString writeText = m_transWriteEdit->text();
    char newWriteSymbol = writeText.isEmpty() ? '_' : writeText.at(0).toLatin1();
    
    Direction newDirection = static_cast<Direction>(m_transDirectionCombo->currentData().toInt());
    
    bool changed = false;
    
    if (newToState != oldToState) {
        m_selectedTransition->setToState(newToState);
        changed = true;
    }
    
    if (newWriteSymbol != oldWriteSymbol) {
        m_selectedTransition->setWriteSymbol(newWriteSymbol);
        changed = true;
    }
    
    if (newDirection != oldDirection) {
        m_selectedTransition->setDirection(newDirection);
        changed = true;
    }
    
    if (changed) {
        emit transitionPropertiesChanged(fromState, readSymbol);
    }
}