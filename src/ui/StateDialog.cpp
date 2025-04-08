#include "StateDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRegularExpressionValidator>

StateDialog::StateDialog(QWidget *parent, State* existingState)
    : QDialog(parent), existingState(existingState)
{
    setWindowTitle(existingState ? tr("Edit State") : tr("Add New State"));
    
    // Create form layout
    QFormLayout *formLayout = new QFormLayout();
    
    // State ID
    idEdit = new QLineEdit(this);
    idEdit->setPlaceholderText(tr("e.g., q0, q1, halt"));
    // Only allow alphanumeric characters and underscores for the ID
    QRegularExpression idRegex("[a-zA-Z0-9_]+");
    idEdit->setValidator(new QRegularExpressionValidator(idRegex, this));
    formLayout->addRow(tr("State &ID:"), idEdit);
    
    // State Name (optional, for display)
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText(tr("Optional display name"));
    formLayout->addRow(tr("State &Name:"), nameEdit);
    
    // State Type
    typeComboBox = new QComboBox(this);
    typeComboBox->addItem(tr("Normal"), static_cast<int>(StateType::NORMAL));
    typeComboBox->addItem(tr("Start"), static_cast<int>(StateType::START));
    typeComboBox->addItem(tr("Accept"), static_cast<int>(StateType::ACCEPT));
    typeComboBox->addItem(tr("Reject"), static_cast<int>(StateType::REJECT));
    formLayout->addRow(tr("State &Type:"), typeComboBox);
    
    // Button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    
    // If we're editing an existing state, fill the form
    if (existingState) {
        idEdit->setText(QString::fromStdString(existingState->getId()));
        idEdit->setEnabled(false);  // Don't allow changing ID of existing state
        nameEdit->setText(QString::fromStdString(existingState->getName()));
        typeComboBox->setCurrentIndex(typeComboBox->findData(static_cast<int>(existingState->getType())));
    }
    
    // Set the initial focus
    idEdit->setFocus();
}

QString StateDialog::getStateId() const
{
    return idEdit->text();
}

QString StateDialog::getStateName() const
{
    return nameEdit->text();
}

StateType StateDialog::getStateType() const
{
    return static_cast<StateType>(typeComboBox->currentData().toInt());
}