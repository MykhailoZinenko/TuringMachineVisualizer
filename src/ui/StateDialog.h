#ifndef STATEDIALOG_H
#define STATEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include "../model/State.h"

class StateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StateDialog(QWidget *parent = nullptr, State* existingState = nullptr);
    
    // Getters for the state properties
    QString getStateId() const;
    QString getStateName() const;
    StateType getStateType() const;

private:
    QLineEdit *idEdit;
    QLineEdit *nameEdit;
    QComboBox *typeComboBox;
    QDialogButtonBox *buttonBox;
    
    State* existingState;
};

#endif // STATEDIALOG_H