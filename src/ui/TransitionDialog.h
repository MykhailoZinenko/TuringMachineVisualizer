#pragma once

#include <QDialog>

// Forward declarations
class QComboBox;
class QLineEdit;
class QDialogButtonBox;
class QFormLayout;
class Transition;
class TuringMachine;

enum class Direction;

class TransitionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransitionDialog(TuringMachine* machine, QWidget *parent = nullptr,
                             Transition* existingTransition = nullptr);

    QString getFromState() const;
    QString getToState() const;
    QChar getReadSymbol() const;
    QChar getWriteSymbol() const;
    Direction getDirection() const;

private:
    QComboBox *fromStateComboBox;
    QComboBox *toStateComboBox;
    QLineEdit *readSymbolEdit;
    QLineEdit *writeSymbolEdit;
    QComboBox *directionComboBox;
    QDialogButtonBox *buttonBox;

    Transition* existingTransition;
};