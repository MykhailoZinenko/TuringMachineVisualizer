#include "TransitionsListWidget.h"
#include "TransitionDialog.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QHeaderView>

TransitionsListWidget::TransitionsListWidget(TuringMachine* machine, QWidget *parent)
    : QWidget(parent), machine(machine)
{
    setupUI();
    refreshTransitionsList();
}

void TransitionsListWidget::setMachine(TuringMachine* newMachine)
{
    machine = newMachine;
    refreshTransitionsList(); // Refresh the list with the new machine
}

void TransitionsListWidget::refreshTransitionsList()
{
    transitionsTable->clearContents();

    // Ensure we have a valid machine pointer
    if (!machine) {
        transitionsTable->setRowCount(0);
        updateButtons();
        return;
    }

    auto transitions = machine->getAllTransitions();
    transitionsTable->setRowCount(transitions.size());

    for (size_t i = 0; i < transitions.size(); ++i) {
        Transition* transition = transitions[i];

        // From state
        transitionsTable->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(transition->getFromState())));

        // Read symbol
        transitionsTable->setItem(i, 1, new QTableWidgetItem(
            QString(transition->getReadSymbol())));

        // To state
        transitionsTable->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(transition->getToState())));

        // Write symbol
        transitionsTable->setItem(i, 3, new QTableWidgetItem(
            QString(transition->getWriteSymbol())));

        // Direction
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

        // Store transition information in item data
        for (int col = 0; col < 5; ++col) {
            transitionsTable->item(i, col)->setData(Qt::UserRole,
                QStringList() << QString::fromStdString(transition->getFromState())
                             << QString(transition->getReadSymbol()));
        }
    }

    updateButtons();
}

void TransitionsListWidget::addTransition()
{
    // Ensure we have a valid machine pointer
    if (!machine) {
        QMessageBox::warning(this, tr("Error"),
            tr("No machine available. Please create a new machine first."));
        return;
    }

    // Make sure we have at least one state
    if (machine->getAllStates().empty()) {
        QMessageBox::warning(this, tr("No States"),
            tr("You need to create at least one state before adding transitions."));
        return;
    }

    TransitionDialog dialog(machine, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::string fromState = dialog.getFromState().toStdString();
        char readSymbol = dialog.getReadSymbol().toLatin1();

        // Check if a transition for this state and symbol already exists
        if (machine->getTransition(fromState, readSymbol)) {
            QMessageBox::warning(this, tr("Duplicate Transition"),
                tr("A transition for state '%1' and symbol '%2' already exists.")
                  .arg(dialog.getFromState())
                  .arg(dialog.getReadSymbol()));
            return;
        }

        // Add the transition to the machine
        machine->addTransition(
            fromState,
            readSymbol,
            dialog.getToState().toStdString(),
            dialog.getWriteSymbol().toLatin1(),
            dialog.getDirection()
        );

        // Refresh the list and emit signal
        refreshTransitionsList();
        emit transitionAdded();
    }
}

void TransitionsListWidget::setupUI()
{
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create transitions table
    transitionsTable = new QTableWidget(this);
    transitionsTable->setColumnCount(5);
    transitionsTable->setHorizontalHeaderLabels(
        QStringList() << tr("From State") << tr("Read") << tr("To State") << tr("Write") << tr("Move"));
    
    // Configure table
    transitionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transitionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transitionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transitionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Connect signals
    connect(transitionsTable, &QTableWidget::itemSelectionChanged, this, &TransitionsListWidget::updateButtons);
    connect(transitionsTable, &QTableWidget::cellDoubleClicked, this, &TransitionsListWidget::handleCellDoubleClick);
    
    mainLayout->addWidget(transitionsTable);
    
    // Create buttons layout
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    
    // Add button
    addButton = new QPushButton(tr("Add"), this);
    connect(addButton, &QPushButton::clicked, this, &TransitionsListWidget::addTransition);
    buttonsLayout->addWidget(addButton);
    
    // Edit button
    editButton = new QPushButton(tr("Edit"), this);
    connect(editButton, &QPushButton::clicked, this, &TransitionsListWidget::editTransition);
    buttonsLayout->addWidget(editButton);
    
    // Remove button
    removeButton = new QPushButton(tr("Remove"), this);
    connect(removeButton, &QPushButton::clicked, this, &TransitionsListWidget::removeTransition);
    buttonsLayout->addWidget(removeButton);
    
    // Add buttons layout to main layout
    mainLayout->addLayout(buttonsLayout);
    
    // Initialize button states
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
        // Update the transition
        transition->setToState(dialog.getToState().toStdString());
        transition->setWriteSymbol(dialog.getWriteSymbol().toLatin1());
        transition->setDirection(dialog.getDirection());
        
        // Refresh the list and emit signal
        refreshTransitionsList();
        emit transitionEdited();
    }
}

void TransitionsListWidget::removeTransition()
{
    // Get the selected transition
    QModelIndexList selection = transitionsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;
    
    int row = selection.first().row();
    QTableWidgetItem* item = transitionsTable->item(row, 0);
    if (!item) return;
    
    QStringList data = item->data(Qt::UserRole).toStringList();
    if (data.size() < 2) return;
    
    std::string fromState = data[0].toStdString();
    char readSymbol = data[1].isEmpty() ? '_' : data[1].at(0).toLatin1();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Deletion"),
        tr("Are you sure you want to delete the transition from state '%1' on symbol '%2'?")
            .arg(QString::fromStdString(fromState))
            .arg(readSymbol),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        machine->removeTransition(fromState, readSymbol);

        // Refresh the list and emit signal
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
    char readSymbol = data[1].isEmpty() ? '_' : data[1].at(0).toLatin1();
    
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
    char readSymbol = data[1].isEmpty() ? '_' : data[1].at(0).toLatin1();

    emit transitionSelected(fromState, readSymbol);
}
