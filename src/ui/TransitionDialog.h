#ifndef TRANSITIONDIALOG_H
#define TRANSITIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include "../model/Transition.h"
#include "../model/TuringMachine.h"

class TransitionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransitionDialog(TuringMachine* machine, QWidget *parent = nullptr, 
                             Transition* existingTransition = nullptr);
    
    // Getters for the transition properties
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

#endif // TRANSITIONDIALOG_H