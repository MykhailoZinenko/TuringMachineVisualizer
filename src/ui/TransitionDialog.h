#pragma once

#include <QDialog>
#include <string>

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
    QString getReadSymbol() const;  // Changed to QString
    QString getWriteSymbol() const; // Changed to QString
    Direction getDirection() const;

private:
    QComboBox *fromStateComboBox;
    QComboBox *toStateComboBox;
    QLineEdit *readSymbolEdit;    // Changed to support multi-symbols
    QLineEdit *writeSymbolEdit;   // Changed to support multi-symbols
    QComboBox *directionComboBox;
    QDialogButtonBox *buttonBox;

    Transition* existingTransition;
};