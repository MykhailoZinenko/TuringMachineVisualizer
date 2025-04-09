#include "StatesListWidget.h"

// Qt includes
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QMessageBox>

// Project includes
#include "../model/TuringMachine.h"
#include "StateDialog.h"

StatesListWidget::StatesListWidget(TuringMachine* machine, QWidget *parent)
    : QWidget(parent), machine(machine)
{
    setupUI();
    refreshStatesList();
}

void StatesListWidget::setMachine(TuringMachine* newMachine)
{
    machine = newMachine;
    refreshStatesList();
}

void StatesListWidget::refreshStatesList()
{
    statesList->clear();

    if (!machine) {
        return;
    }

    auto states = machine->getAllStates();
    for (auto state : states) {
        QString displayText = QString::fromStdString(state->getId());
        if (!state->getName().empty()) {
            displayText += " (" + QString::fromStdString(state->getName()) + ")";
        }

        QListWidgetItem* item = new QListWidgetItem(displayText, statesList);
        item->setData(Qt::UserRole, QString::fromStdString(state->getId()));

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
                break;
        }
    }

    updateButtons();
}

void StatesListWidget::addState()
{
    if (!machine) {
        QMessageBox::warning(this, tr("Error"),
            tr("No machine available. Please create a new machine first."));
        return;
    }

    StateDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        std::string stateId = dialog.getStateId().toStdString();

        if (machine->getState(stateId)) {
            QMessageBox::warning(this, tr("Duplicate State ID"),
                tr("A state with ID '%1' already exists.").arg(dialog.getStateId()));
            return;
        }

        machine->addState(stateId,
                          dialog.getStateName().toStdString(),
                          dialog.getStateType());

        if (dialog.getStateType() == StateType::START) {
            machine->setStartState(stateId);
        }

        refreshStatesList();
        emit stateAdded(stateId);
    }
}

void StatesListWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    statesList = new QListWidget(this);
    connect(statesList, &QListWidget::itemSelectionChanged, this, &StatesListWidget::updateButtons);
    connect(statesList, &QListWidget::itemDoubleClicked, this, &StatesListWidget::editState);
    mainLayout->addWidget(statesList);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    addButton = new QPushButton(tr("Add"), this);
    connect(addButton, &QPushButton::clicked, this, &StatesListWidget::addState);
    buttonsLayout->addWidget(addButton);

    editButton = new QPushButton(tr("Edit"), this);
    connect(editButton, &QPushButton::clicked, this, &StatesListWidget::editState);
    buttonsLayout->addWidget(editButton);

    removeButton = new QPushButton(tr("Remove"), this);
    connect(removeButton, &QPushButton::clicked, this, &StatesListWidget::removeState);
    buttonsLayout->addWidget(removeButton);

    mainLayout->addLayout(buttonsLayout);

    updateButtons();

    connect(statesList, &QListWidget::itemSelectionChanged, this, &StatesListWidget::onStateSelectionChanged);
}

void StatesListWidget::editState()
{
    QListWidgetItem* item = statesList->currentItem();
    if (!item) return;

    std::string stateId = item->data(Qt::UserRole).toString().toStdString();
    State* state = machine->getState(stateId);
    if (!state) return;

    StateDialog dialog(this, state);
    if (dialog.exec() == QDialog::Accepted) {
        state->setName(dialog.getStateName().toStdString());

        StateType newType = dialog.getStateType();
        if (newType != state->getType()) {
            state->setType(newType);

            if (newType == StateType::START) {
                machine->setStartState(stateId);
            }
        }

        refreshStatesList();
        emit stateEdited(stateId);
    }
}

void StatesListWidget::removeState()
{
    QListWidgetItem* item = statesList->currentItem();
    if (!item) return;

    std::string stateId = item->data(Qt::UserRole).toString().toStdString();

    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Deletion"),
        tr("Are you sure you want to delete state '%1'?\nThis will also remove all transitions involving this state.")
            .arg(QString::fromStdString(stateId)),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        std::string removedId = stateId;
        machine->removeState(stateId);

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
            item->setBackground(QColor(255, 235, 185));
        } else {
            item->setBackground(Qt::transparent);
        }
    }
}

void StatesListWidget::onStateSelectionChanged()
{
    QListWidgetItem* item = statesList->currentItem();
    if (item) {
        std::string stateId = item->data(Qt::UserRole).toString().toStdString();
        emit stateSelected(stateId);
    }
}