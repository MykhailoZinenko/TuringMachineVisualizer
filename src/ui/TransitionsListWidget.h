#pragma once

#include <QWidget>
#include <string>

// Forward declarations
class QTableWidget;
class QPushButton;
class TuringMachine;
class Transition;

class TransitionsListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransitionsListWidget(TuringMachine* machine, QWidget *parent = nullptr);

    void setMachine(TuringMachine* machine);
    void refreshTransitionsList();

    signals:
        void transitionAdded();
    void transitionEdited();
    void transitionRemoved();
    void transitionSelected(const std::string& fromState, const std::string& readSymbol);  // Changed to use std::string

    private slots:
        void addTransition();
    void editTransition();
    void removeTransition();
    void updateButtons();
    void handleCellDoubleClick(int row, int column);
    void onTransitionSelectionChanged();

private:
    TuringMachine* machine;

    QTableWidget* transitionsTable;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* removeButton;

    void setupUI();
    Transition* getSelectedTransition();
};