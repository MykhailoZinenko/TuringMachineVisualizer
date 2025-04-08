#ifndef STATESLISTWIDGET_H
#define STATESLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../model/TuringMachine.h"

class StatesListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatesListWidget(TuringMachine* machine, QWidget *parent = nullptr);
    void setMachine(TuringMachine* machine); // New setter to update the pointer
    void refreshStatesList();
    void highlightCurrentState(const std::string& currentStateId);

    signals:
        void stateAdded(const std::string& stateId);
    void stateEdited(const std::string& stateId);
    void stateRemoved(const std::string& stateId);

    private slots:
        void addState();
    void editState();
    void removeState();
    void updateButtons();

private:
    TuringMachine* machine;

    QListWidget* statesList;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* removeButton;

    void setupUI();
};

#endif // STATESLISTWIDGET_H