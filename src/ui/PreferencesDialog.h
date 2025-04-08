#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>

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

#endif // PREFERENCESDIALOG_H