#include "TransitionsListWidget.h"

// Qt includes
#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVariant>

// Project includes
#include "../model/TuringMachine.h"
#include "TransitionDialog.h"

TransitionsListWidget::TransitionsListWidget(TuringMachine* machine, QWidget *parent)
    : QWidget(parent), machine(machine)
{
    setupUI();
    refreshTransitionsList();
}

void TransitionsListWidget::setMachine(TuringMachine* newMachine)
{
    machine = newMachine;
    refreshTransitionsList();
}

void TransitionsListWidget::refreshTransitionsList()
{
    transitionsTable->clearContents();

    if (!machine) {
        transitionsTable->setRowCount(0);
        updateButtons();
        return;
    }

    auto transitions = machine->getAllTransitions();
    transitionsTable->setRowCount(transitions.size());

    for (size_t i = 0; i < transitions.size(); ++i) {
        Transition* transition = transitions[i];

        transitionsTable->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(transition->getFromState())));

        transitionsTable->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(transition->getReadSymbol())));

        transitionsTable->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(transition->getToState())));

        transitionsTable->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(transition->getWriteSymbol())));

        QString dirText;
        switch (transition->getDirection()) {
            case Direction::LEFT:
                dirText = tr("Left");
                break;
            case Direction::RIGHT:
                dirText = tr("Right");
                break;
            case Direction::STAY:
                dirText = tr("Stay");
                break;
        }
        transitionsTable->setItem(i, 4, new QTableWidgetItem(dirText));

        // Store the transition key in user data
        QStringList data;
        data << QString::fromStdString(transition->getFromState())
             << QString::fromStdString(transition->getReadSymbol());

        for (int col = 0; col < 5; ++col) {
            transitionsTable->item(i, col)->setData(Qt::UserRole, data);
        }
    }

    updateButtons();
}

void TransitionsListWidget::addTransition()
{
    if (!machine) {
        QMessageBox::warning(this, tr("Error"),
            tr("No machine available. Please create a new machine first."));
        return;
    }

    if (machine->getAllStates().empty()) {
        QMessageBox::warning(this, tr("No States"),
            tr("You need to create at least one state before adding transitions."));
        return;
    }

    TransitionDialog dialog(machine, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::string fromState = dialog.getFromState().toStdString();
        std::string readSymbol = dialog.getReadSymbol().toStdString();

        if (machine->getTransition(fromState, readSymbol)) {
            QMessageBox::warning(this, tr("Duplicate Transition"),
                tr("A transition for state '%1' and symbol '%2' already exists.")
                  .arg(dialog.getFromState())
                  .arg(dialog.getReadSymbol()));
            return;
        }

        machine->addTransition(
            fromState,
            readSymbol,
            dialog.getToState().toStdString(),
            dialog.getWriteSymbol().toStdString(),
            dialog.getDirection()
        );

        refreshTransitionsList();
        emit transitionAdded();
    }
}

void TransitionsListWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    transitionsTable = new QTableWidget(this);
    transitionsTable->setColumnCount(5);
    transitionsTable->setHorizontalHeaderLabels(
        QStringList() << tr("From State") << tr("Read") << tr("To State") << tr("Write") << tr("Move"));

    transitionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transitionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transitionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transitionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(transitionsTable, &QTableWidget::itemSelectionChanged, this, &TransitionsListWidget::updateButtons);
    connect(transitionsTable, &QTableWidget::cellDoubleClicked, this, &TransitionsListWidget::handleCellDoubleClick);

    mainLayout->addWidget(transitionsTable);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    addButton = new QPushButton(tr("Add"), this);
    connect(addButton, &QPushButton::clicked, this, &TransitionsListWidget::addTransition);
    buttonsLayout->addWidget(addButton);

    editButton = new QPushButton(tr("Edit"), this);
    connect(editButton, &QPushButton::clicked, this, &TransitionsListWidget::editTransition);
    buttonsLayout->addWidget(editButton);

    removeButton = new QPushButton(tr("Remove"), this);
    connect(removeButton, &QPushButton::clicked, this, &TransitionsListWidget::removeTransition);
    buttonsLayout->addWidget(removeButton);

    mainLayout->addLayout(buttonsLayout);

    updateButtons();

    connect(transitionsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &TransitionsListWidget::onTransitionSelectionChanged);
}

void TransitionsListWidget::editTransition()
{
    Transition* transition = getSelectedTransition();
    if (!transition) return;

    TransitionDialog dialog(machine, this, transition);
    if (dialog.exec() == QDialog::Accepted) {
        transition->setToState(dialog.getToState().toStdString());
        transition->setWriteSymbol(dialog.getWriteSymbol().toStdString());
        transition->setDirection(dialog.getDirection());

        refreshTransitionsList();
        emit transitionEdited();
    }
}

void TransitionsListWidget::removeTransition()
{
    QModelIndexList selection = transitionsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    int row = selection.first().row();
    QTableWidgetItem* item = transitionsTable->item(row, 0);
    if (!item) return;

    QStringList data = item->data(Qt::UserRole).toStringList();
    if (data.size() < 2) return;

    std::string fromState = data[0].toStdString();
    std::string readSymbol = data[1].toStdString();

    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Deletion"),
        tr("Are you sure you want to delete the transition from state '%1' on symbol '%2'?")
            .arg(QString::fromStdString(fromState))
            .arg(QString::fromStdString(readSymbol)),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        machine->removeTransition(fromState, readSymbol);
        refreshTransitionsList();
        emit transitionRemoved();
    }
}

void TransitionsListWidget::updateButtons()
{
    bool hasSelection = !transitionsTable->selectionModel()->selectedRows().isEmpty();
    editButton->setEnabled(hasSelection);
    removeButton->setEnabled(hasSelection);
}

void TransitionsListWidget::handleCellDoubleClick(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    editTransition();
}

Transition* TransitionsListWidget::getSelectedTransition()
{
    QModelIndexList selection = transitionsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) return nullptr;

    int row = selection.first().row();
    QTableWidgetItem* item = transitionsTable->item(row, 0);
    if (!item) return nullptr;

    QStringList data = item->data(Qt::UserRole).toStringList();
    if (data.size() < 2) return nullptr;

    std::string fromState = data[0].toStdString();
    std::string readSymbol = data[1].toStdString();

    return machine->getTransition(fromState, readSymbol);
}

void TransitionsListWidget::onTransitionSelectionChanged()
{
    QModelIndexList selection = transitionsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    int row = selection.first().row();
    QTableWidgetItem* item = transitionsTable->item(row, 0);
    if (!item) return;

    QStringList data = item->data(Qt::UserRole).toStringList();
    if (data.size() < 2) return;

    std::string fromState = data[0].toStdString();
    std::string readSymbol = data[1].toStdString();

    emit transitionSelected(fromState, readSymbol);
}