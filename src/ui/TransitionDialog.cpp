#include "TransitionDialog.h"

// Qt includes
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

// Project includes
#include "../model/Transition.h"
#include "../model/TuringMachine.h"

TransitionDialog::TransitionDialog(TuringMachine* machine, QWidget *parent, Transition* existingTransition)
    : QDialog(parent), existingTransition(existingTransition)
{
    setWindowTitle(existingTransition ? tr("Edit Transition") : tr("Add New Transition"));

    QFormLayout *formLayout = new QFormLayout();

    // From State
    fromStateComboBox = new QComboBox(this);
    auto states = machine->getAllStates();
    for (auto state : states) {
        QString stateText = QString::fromStdString(state->getId());
        if (!state->getName().empty()) {
            stateText += " (" + QString::fromStdString(state->getName()) + ")";
        }
        fromStateComboBox->addItem(stateText, QString::fromStdString(state->getId()));
    }
    formLayout->addRow(tr("&From State:"), fromStateComboBox);

    // Read Symbol - now allows multiple characters
    readSymbolEdit = new QLineEdit(this);
    readSymbolEdit->setMaxLength(10); // Allow more symbols
    formLayout->addRow(tr("&Read Symbol:"), readSymbolEdit);

    // Add a hint label for multi-symbol support
    QLabel* readHintLabel = new QLabel(tr("Use 'Blank' for blank symbol, multiple symbols allowed"), this);
    readHintLabel->setStyleSheet("color: gray; font-size: 11px;");
    formLayout->addRow("", readHintLabel);

    // To State
    toStateComboBox = new QComboBox(this);
    for (auto state : states) {
        QString stateText = QString::fromStdString(state->getId());
        if (!state->getName().empty()) {
            stateText += " (" + QString::fromStdString(state->getName()) + ")";
        }
        toStateComboBox->addItem(stateText, QString::fromStdString(state->getId()));
    }
    formLayout->addRow(tr("&To State:"), toStateComboBox);

    // Write Symbol - now allows multiple characters
    writeSymbolEdit = new QLineEdit(this);
    writeSymbolEdit->setMaxLength(10); // Allow more symbols
    formLayout->addRow(tr("&Write Symbol:"), writeSymbolEdit);

    // Add a hint label for multi-symbol support
    QLabel* writeHintLabel = new QLabel(tr("Use 'Blank' for blank symbol, multiple symbols allowed"), this);
    writeHintLabel->setStyleSheet("color: gray; font-size: 11px;");
    formLayout->addRow("", writeHintLabel);

    // Direction
    directionComboBox = new QComboBox(this);
    directionComboBox->addItem(tr("Left (L)"), static_cast<int>(Direction::LEFT));
    directionComboBox->addItem(tr("Right (R)"), static_cast<int>(Direction::RIGHT));
    directionComboBox->addItem(tr("Stay (N)"), static_cast<int>(Direction::STAY));
    directionComboBox->setCurrentIndex(1);  // Default to RIGHT
    formLayout->addRow(tr("&Direction:"), directionComboBox);

    // Button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    // If editing existing transition, fill the form
    if (existingTransition) {
        int fromIndex = fromStateComboBox->findData(QString::fromStdString(existingTransition->getFromState()));
        if (fromIndex >= 0) {
            fromStateComboBox->setCurrentIndex(fromIndex);
        }

        readSymbolEdit->setText(QString::fromStdString(existingTransition->getReadSymbol()));

        int toIndex = toStateComboBox->findData(QString::fromStdString(existingTransition->getToState()));
        if (toIndex >= 0) {
            toStateComboBox->setCurrentIndex(toIndex);
        }

        writeSymbolEdit->setText(QString::fromStdString(existingTransition->getWriteSymbol()));

        int dirIndex = directionComboBox->findData(static_cast<int>(existingTransition->getDirection()));
        if (dirIndex >= 0) {
            directionComboBox->setCurrentIndex(dirIndex);
        }

        fromStateComboBox->setEnabled(false);
        readSymbolEdit->setEnabled(false);
    }

    if (!existingTransition) {
        fromStateComboBox->setFocus();
    } else {
        toStateComboBox->setFocus();
    }
}

QString TransitionDialog::getFromState() const
{
    return fromStateComboBox->currentData().toString();
}

QString TransitionDialog::getToState() const
{
    return toStateComboBox->currentData().toString();
}

QString TransitionDialog::getReadSymbol() const
{
    QString text = readSymbolEdit->text().trimmed();

    // Convert "Blank" to blank symbol
    if (text.toLower() == "blank") {
        return "_";
    }

    return text.isEmpty() ? "_" : text;
}

QString TransitionDialog::getWriteSymbol() const
{
    QString text = writeSymbolEdit->text().trimmed();

    // Convert "Blank" to blank symbol
    if (text.toLower() == "blank") {
        return "_";
    }

    return text.isEmpty() ? "_" : text;
}

Direction TransitionDialog::getDirection() const
{
    return static_cast<Direction>(directionComboBox->currentData().toInt());
}