#pragma once

#include <QDialog>

// Forward declarations
class QLineEdit;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class State;

enum class StateType;

class StateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StateDialog(QWidget *parent = nullptr, State* existingState = nullptr);

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