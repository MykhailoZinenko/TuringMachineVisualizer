#include "StatesListWidget.h"
#include "StateDialog.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QIcon>

StatesListWidget::StatesListWidget(TuringMachine* machine, QWidget *parent)
    : QWidget(parent), machine(machine)
{
    setupUI();
    refreshStatesList();
}

void StatesListWidget::setMachine(TuringMachine* newMachine)
{
    machine = newMachine;
    refreshStatesList(); // Refresh the list with the new machine
}

void StatesListWidget::refreshStatesList()
{
    // Clear the list first
    statesList->clear();

    // Ensure we have a valid machine pointer
    if (!machine) {
        return;
    }

    // Get all states and populate the list
    auto states = machine->getAllStates();
    for (auto state : states) {
        QString displayText = QString::fromStdString(state->getId());
        if (!state->getName().empty()) {
            displayText += " (" + QString::fromStdString(state->getName()) + ")";
        }

        QListWidgetItem* item = new QListWidgetItem(displayText, statesList);
        item->setData(Qt::UserRole, QString::fromStdString(state->getId()));

        // Set icon based on state type
        switch (state->getType()) {
            case StateType::START:
                item->setIcon(QIcon::fromTheme("media-playback-start"));
                break;
            case StateType::ACCEPT:
                item->setIcon(QIcon::fromTheme("dialog-ok"));
                break;
            case StateType::REJECT:
                item->setIcon(QIcon::fromTheme("dialog-cancel"));
                break;
            default:
                // No icon for normal states
                break;
        }
    }

    updateButtons();
}

void StatesListWidget::addState()
{
    // Ensure we have a valid machine pointer
    if (!machine) {
        QMessageBox::warning(this, tr("Error"),
            tr("No machine available. Please create a new machine first."));
        return;
    }

    StateDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        std::string stateId = dialog.getStateId().toStdString();

        // Check if state with this ID already exists
        if (machine->getState(stateId)) {
            QMessageBox::warning(this, tr("Duplicate State ID"),
                tr("A state with ID '%1' already exists.").arg(dialog.getStateId()));
            return;
        }

        // Add the state to the machine
        machine->addState(stateId,
                          dialog.getStateName().toStdString(),
                          dialog.getStateType());

        // If this is a start state, set it as the machine's start state
        if (dialog.getStateType() == StateType::START) {
            machine->setStartState(stateId);
        }

        // Refresh the list and emit signal
        refreshStatesList();
        emit stateAdded(stateId);
    }
}

void StatesListWidget::setupUI()
{
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create states list
    statesList = new QListWidget(this);
    connect(statesList, &QListWidget::itemSelectionChanged, this, &StatesListWidget::updateButtons);
    connect(statesList, &QListWidget::itemDoubleClicked, this, &StatesListWidget::editState);
    mainLayout->addWidget(statesList);
    
    // Create buttons layout
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    
    // Add button
    addButton = new QPushButton(tr("Add"), this);
    connect(addButton, &QPushButton::clicked, this, &StatesListWidget::addState);
    buttonsLayout->addWidget(addButton);
    
    // Edit button
    editButton = new QPushButton(tr("Edit"), this);
    connect(editButton, &QPushButton::clicked, this, &StatesListWidget::editState);
    buttonsLayout->addWidget(editButton);
    
    // Remove button
    removeButton = new QPushButton(tr("Remove"), this);
    connect(removeButton, &QPushButton::clicked, this, &StatesListWidget::removeState);
    buttonsLayout->addWidget(removeButton);
    
    // Add buttons layout to main layout
    mainLayout->addLayout(buttonsLayout);
    
    // Initialize button states
    updateButtons();
}

void StatesListWidget::editState()
{
    // Get the selected state
    QListWidgetItem* item = statesList->currentItem();
    if (!item) return;
    
    std::string stateId = item->data(Qt::UserRole).toString().toStdString();
    State* state = machine->getState(stateId);
    if (!state) return;
    
    // Show the dialog with the state's current values
    StateDialog dialog(this, state);
    if (dialog.exec() == QDialog::Accepted) {
        // Update the state
        state->setName(dialog.getStateName().toStdString());
        
        // Handle state type changes
        StateType newType = dialog.getStateType();
        if (newType != state->getType()) {
            state->setType(newType);
            
            // If changed to START, update the machine's start state
            if (newType == StateType::START) {
                machine->setStartState(stateId);
            }
        }
        
        // Refresh the list and emit signal
        refreshStatesList();
        emit stateEdited(stateId);
    }
}

void StatesListWidget::removeState()
{
    // Get the selected state
    QListWidgetItem* item = statesList->currentItem();
    if (!item) return;
    
    std::string stateId = item->data(Qt::UserRole).toString().toStdString();
    
    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Deletion"),
        tr("Are you sure you want to delete state '%1'?\nThis will also remove all transitions involving this state.")
            .arg(QString::fromStdString(stateId)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        std::string removedId = stateId;
        machine->removeState(stateId);
        
        // Refresh the list and emit signal
        refreshStatesList();
        emit stateRemoved(removedId);
    }
}

void StatesListWidget::updateButtons()
{
    bool hasSelection = statesList->currentItem() != nullptr;
    editButton->setEnabled(hasSelection);
    removeButton->setEnabled(hasSelection);
}

void StatesListWidget::highlightCurrentState(const std::string& currentStateId)
{
    for (int i = 0; i < statesList->count(); ++i) {
        QListWidgetItem* item = statesList->item(i);
        QString stateId = item->data(Qt::UserRole).toString();
        if (stateId.toStdString() == currentStateId) {
            statesList->setCurrentItem(item);
            item->setBackground(QColor(255, 235, 185)); // Light orange highlight
        } else {
            item->setBackground(Qt::transparent);
        }
    }
}
