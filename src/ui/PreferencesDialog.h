#pragma once

#include <QDialog>

// Forward declarations
class QSpinBox;
class QDialogButtonBox;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(int currentSpeed, QWidget *parent = nullptr);
    int getSimulationSpeed() const;

private:
    QSpinBox* speedSpinBox;
    QDialogButtonBox* buttonBox;
};