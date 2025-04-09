#include "PreferencesDialog.h"

// Qt includes
#include <QFormLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QDialogButtonBox>

PreferencesDialog::PreferencesDialog(int currentSpeed, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    
    QFormLayout* formLayout = new QFormLayout();
    
    speedSpinBox = new QSpinBox(this);
    speedSpinBox->setRange(50, 1000);
    speedSpinBox->setValue(currentSpeed);
    speedSpinBox->setSingleStep(50);
    formLayout->addRow(tr("Simulation Speed (ms/step):"), speedSpinBox);
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

int PreferencesDialog::getSimulationSpeed() const
{
    return speedSpinBox->value();
}