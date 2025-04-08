#ifndef TRANSITIONSLISTWIDGET_H
#define TRANSITIONSLISTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../model/TuringMachine.h"

class TransitionsListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransitionsListWidget(TuringMachine* machine, QWidget *parent = nullptr);
    void setMachine(TuringMachine* machine); // New setter to update the pointer
    void refreshTransitionsList();

    signals:
        void transitionAdded();
    void transitionEdited();
    void transitionRemoved();
    void transitionSelected(const std::string& fromState, char readSymbol); // New signal for transition selection

    private slots:
        void addTransition();
    void editTransition();
    void removeTransition();
    void updateButtons();
    void handleCellDoubleClick(int row, int column);
    void onTransitionSelectionChanged(); // New slot for handling selection changes

private:
    TuringMachine* machine;

    QTableWidget* transitionsTable;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* removeButton;

    void setupUI();
    Transition* getSelectedTransition();
};

#endif // TRANSITIONSLISTWIDGET_H