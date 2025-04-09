#pragma once

#include <QWidget>
#include <string>

// Forward declarations
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QVBoxLayout;
class TuringMachine;

class StatesListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatesListWidget(TuringMachine* machine, QWidget *parent = nullptr);

    void setMachine(TuringMachine* machine);
    void refreshStatesList();
    void highlightCurrentState(const std::string& currentStateId);

    signals:
        void stateAdded(const std::string& stateId);
    void stateEdited(const std::string& stateId);
    void stateRemoved(const std::string& stateId);
    void stateSelected(const std::string& stateId);

    private slots:
        void addState();
    void editState();
    void removeState();
    void updateButtons();
    void onStateSelectionChanged();

private:
    TuringMachine* machine;

    QListWidget* statesList;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* removeButton;

    void setupUI();
};