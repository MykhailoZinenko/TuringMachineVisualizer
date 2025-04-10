#include "TuringMachine.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <QtCore/qstring.h>
#include <qdebug.h>

using json = nlohmann::json;

// Constructor & destructor
TuringMachine::TuringMachine(const std::string& name, MachineType type)
    : name(name), type(type), status(ExecutionStatus::READY),
      stepCount(0), maxHistorySize(1000), historyPosition(-1)
{
    tape = std::make_unique<Tape>();
}

TuringMachine::~TuringMachine()
{
}

// Machine configuration
std::string TuringMachine::getName() const
{
    return name;
}

void TuringMachine::setName(const std::string& name)
{
    this->name = name;
}

MachineType TuringMachine::getType() const
{
    return type;
}

void TuringMachine::setType(MachineType type)
{
    this->type = type;
}

// State management
void TuringMachine::addState(const std::string& id, const std::string& name, StateType type)
{
    if (states.find(id) == states.end()) {
        states[id] = std::make_unique<State>(id, name, type);

        if (states.size() == 1 || type == StateType::START) {
            currentState = id;
        }
    }
}

void TuringMachine::removeState(const std::string& id)
{
    states.erase(id);

    auto it = transitions.begin();
    while (it != transitions.end()) {
        if (it->first.first == id || it->second->getToState() == id) {
            it = transitions.erase(it);
        } else {
            ++it;
        }
    }

    if (currentState == id) {
        if (!states.empty()) {
            auto startIt = std::find_if(states.begin(), states.end(),
                [](const auto& pair) { return pair.second->getType() == StateType::START; });

            if (startIt != states.end()) {
                currentState = startIt->first;
            } else {
                currentState = states.begin()->first;
            }
        } else {
            currentState = "";
        }
    }
}

State* TuringMachine::getState(const std::string& id)
{
    auto it = states.find(id);
    if (it != states.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<State*> TuringMachine::getAllStates() const
{
    std::vector<State*> result;
    for (const auto& pair : states) {
        result.push_back(pair.second.get());
    }
    return result;
}

std::string TuringMachine::getStartState() const
{
    for (const auto& pair : states) {
        if (pair.second->getType() == StateType::START) {
            return pair.first;
        }
    }
    return "";
}

void TuringMachine::setStartState(const std::string& id)
{
    State* state = getState(id);
    if (state) {
        for (auto& pair : states) {
            if (pair.second->getType() == StateType::START) {
                pair.second->setType(StateType::NORMAL);
            }
        }

        state->setType(StateType::START);

        if (status == ExecutionStatus::READY) {
            currentState = id;
        }
    }
}

// Transition management - updated to use strings instead of chars
void TuringMachine::addTransition(const std::string& fromState, const std::string& readSymbol,
                               const std::string& toState, const std::string& writeSymbol,
                               Direction moveDirection)
{
    if (getState(fromState) && getState(toState)) {
        std::pair<std::string, std::string> key(fromState, readSymbol);
        transitions[key] = std::make_unique<Transition>(
            fromState, readSymbol, toState, writeSymbol, moveDirection);
    }
}

void TuringMachine::removeTransition(const std::string& fromState, const std::string& readSymbol)
{
    std::pair<std::string, std::string> key(fromState, readSymbol);
    transitions.erase(key);
}

Transition* TuringMachine::getTransition(const std::string& fromState, const std::string& readSymbol)
{
    std::pair<std::string, std::string> key(fromState, readSymbol);
    auto it = transitions.find(key);
    if (it != transitions.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Transition*> TuringMachine::getAllTransitions() const
{
    std::vector<Transition*> result;
    for (const auto& pair : transitions) {
        result.push_back(pair.second.get());
    }
    return result;
}

// Tape management
Tape* TuringMachine::getTape()
{
    return tape.get();
}

void TuringMachine::setTapeContent(const std::string& content)
{
    tape->setInitialContent(content);
}

std::string TuringMachine::getTapeContent() const
{
    return tape->getCurrentContent(50);
}

// Execution control
void TuringMachine::reset()
{
    if (!getStartState().empty()) {
        currentState = getStartState();
    } else if (!states.empty()) {
        currentState = states.begin()->first;
    } else {
        currentState = "";
    }

    tape->reset();
    status = ExecutionStatus::READY;
    stepCount = 0;
    clearHistory();

    addToHistory(createSnapshot());
}

bool TuringMachine::step()
{
    if (status == ExecutionStatus::HALTED_ACCEPT ||
        status == ExecutionStatus::HALTED_REJECT ||
        status == ExecutionStatus::ERROR) {
        return false;
    }

    status = ExecutionStatus::RUNNING;

    State* state = getState(currentState);
    if (!state) {
        status = ExecutionStatus::ERROR;
        return false;
    }

    if (state->isAcceptState()) {
        status = ExecutionStatus::HALTED_ACCEPT;
        return false;
    }

    if (state->isRejectState()) {
        status = ExecutionStatus::HALTED_REJECT;
        return false;
    }

    std::string symbol = tape->read();
    Transition* transition = getTransition(currentState, symbol);

    if (!transition) {
        // Try with the blank symbol as a fallback
        transition = getTransition(currentState, std::string(1, tape->getBlankSymbol()));
    }

    if (!transition) {
        status = ExecutionStatus::ERROR;
        qDebug() << "Error: No transition found for state" << QString::fromStdString(currentState)
                 << "and symbol" << QString::fromStdString(symbol);
        return false;
    }

    tape->write(transition->getWriteSymbol());

    switch (transition->getDirection()) {
        case Direction::LEFT:
            tape->moveLeft();
            break;
        case Direction::RIGHT:
            tape->moveRight();
            break;
        case Direction::STAY:
            break;
    }

    currentState = transition->getToState();

    stepCount++;
    addToHistory(createSnapshot());

    return true;
}

void TuringMachine::run()
{
    status = ExecutionStatus::RUNNING;
}

void TuringMachine::pause()
{
    if (status == ExecutionStatus::RUNNING) {
        status = ExecutionStatus::PAUSED;
    }
}

bool TuringMachine::canStepBackward() const
{
    return historyPosition > 0;
}

bool TuringMachine::stepBackward()
{
    if (!canStepBackward()) {
        return false;
    }

    historyPosition--;
    restoreSnapshot(history[historyPosition]);
    stepCount--;

    if (historyPosition == 0) {
        status = ExecutionStatus::READY;
    } else {
        status = ExecutionStatus::PAUSED;
    }

    return true;
}

ExecutionStatus TuringMachine::getStatus() const
{
    return status;
}

std::string TuringMachine::getCurrentState() const
{
    return currentState;
}

// Analysis and statistics
int TuringMachine::getStepCount() const
{
    return stepCount;
}

int TuringMachine::getMaxHistorySize() const
{
    return maxHistorySize;
}

void TuringMachine::setMaxHistorySize(int size)
{
    maxHistorySize = size;

    if (history.size() > static_cast<size_t>(maxHistorySize)) {
        int toRemove = history.size() - maxHistorySize;
        history.erase(history.begin(), history.begin() + toRemove);
        historyPosition -= toRemove;
    }
}

// Serialization
std::string TuringMachine::toJson() const
{
    json j;
    j["name"] = name;
    j["type"] = static_cast<int>(type);
    j["currentState"] = currentState;
    j["tapeContent"] = tape->getCurrentContent();

    json statesJson = json::array();
    for (const auto& pair : states) {
        statesJson.push_back(json{
            {"id", pair.second->getId()},
            {"name", pair.second->getName()},
            {"type", static_cast<int>(pair.second->getType())},
            {"posX", pair.second->getPosition().x()},
            {"posY", pair.second->getPosition().y()}
        });
    }
    j["states"] = statesJson;

    json transitionsJson = json::array();
    for (const auto& pair : transitions) {
        transitionsJson.push_back(json{
            {"fromState", pair.second->getFromState()},
            {"readSymbol", pair.second->getReadSymbol()},
            {"toState", pair.second->getToState()},
            {"writeSymbol", pair.second->getWriteSymbol()},
            {"direction", static_cast<int>(pair.second->getDirection())}
        });
    }
    j["transitions"] = transitionsJson;

    qDebug() << "Saving machine with:" << states.size() << "states and" << transitions.size() << "transitions";

    return j.dump(4);
}

std::unique_ptr<TuringMachine> TuringMachine::fromJson(const std::string& jsonStr)
{
    qDebug() << "Parsing JSON string of length:" << jsonStr.length();

    try {
        json j = json::parse(jsonStr);

        auto machine = std::make_unique<TuringMachine>(j.value("name", "Untitled"),
                                                      static_cast<MachineType>(j.value("type", 0)));

        if (j.contains("states") && j["states"].is_array()) {
            qDebug() << "Found states array with" << j["states"].size() << "states";

            for (const auto& stateJson : j["states"]) {
                try {
                    std::string id = stateJson["id"];
                    std::string name = stateJson["name"];
                    StateType type = static_cast<StateType>(stateJson["type"].get<int>());

                    qDebug() << "Adding state:" << QString::fromStdString(id);
                    machine->addState(id, name, type);

                    State* state = machine->getState(id);
                    if (state) {
                        state->setPosition(Point2D(stateJson["posX"].get<float>(),
                                                 stateJson["posY"].get<float>()));
                    }
                } catch (const std::exception& e) {
                    qWarning() << "Error restoring state:" << e.what();
                }
            }
        } else {
            qWarning() << "No states array found in JSON";
        }

        int transitionCount = 0;
        if (j.contains("transitions") && j["transitions"].is_array()) {
            qDebug() << "Found transitions array with" << j["transitions"].size() << "transitions";

            for (const auto& transJson : j["transitions"]) {
                try {
                    std::string fromState = transJson["fromState"];
                    std::string readSymbol;

                    // Handle readSymbol which could be a string or a single character in older files
                    if (transJson["readSymbol"].is_string()) {
                        readSymbol = transJson["readSymbol"].get<std::string>();
                    } else if (transJson["readSymbol"].is_number()) {
                        // For backwards compatibility with older files that stored a char as a number
                        char readChar = static_cast<char>(transJson["readSymbol"].get<int>());
                        readSymbol = std::string(1, readChar);
                    } else {
                        // Default
                        readSymbol = "_";
                    }

                    std::string toState = transJson["toState"];
                    std::string writeSymbol;

                    // Handle writeSymbol which could be a string or a single character
                    if (transJson["writeSymbol"].is_string()) {
                        writeSymbol = transJson["writeSymbol"].get<std::string>();
                    } else if (transJson["writeSymbol"].is_number()) {
                        // For backwards compatibility
                        char writeChar = static_cast<char>(transJson["writeSymbol"].get<int>());
                        writeSymbol = std::string(1, writeChar);
                    } else {
                        // Default
                        writeSymbol = "_";
                    }

                    Direction direction = static_cast<Direction>(transJson["direction"].get<int>());

                    qDebug() << "Adding transition from" << QString::fromStdString(fromState) << "on" << QString::fromStdString(readSymbol);

                    machine->addTransition(
                        fromState,
                        readSymbol,
                        toState,
                        writeSymbol,
                        direction
                    );

                    transitionCount++;
                    qDebug() << "TuringMachine::addTransition" << transitionCount;
                } catch (const std::exception& e) {
                    qWarning() << "Error restoring transition:" << e.what();
                }
            }
        } else {
            qWarning() << "No transitions array found in JSON";
        }

        machine->setTapeContent(j.value("tapeContent", ""));
        machine->currentState = j.value("currentState", machine->getStartState());

        qDebug() << "Machine loaded with" << machine->getAllStates().size() << "states and"
                 << machine->getAllTransitions().size() << "transitions";

        // Final check to ensure transitions are still there
        auto allTransitions = machine->getAllTransitions();
        qDebug() << "Turing Machine loaded::load" << allTransitions;

        return machine;
    } catch (const json::exception& e) {
        qCritical() << "JSON parsing error:" << e.what();
        throw;
    } catch (const std::exception& e) {
        qCritical() << "General error in fromJson:" << e.what();
        throw;
    }
}

// Helper methods
ExecutionSnapshot TuringMachine::createSnapshot() const
{
    ExecutionSnapshot snapshot;
    snapshot.currentState = currentState;
    snapshot.headPosition = tape->getHeadPosition();

    // Get the tape content
    int left = tape->getLeftmostUsedPosition();
    int right = tape->getRightmostUsedPosition();

    auto visibleCells = tape->getVisiblePortion(left, right - left + 1);
    for (const auto& cell : visibleCells) {
        if (cell.second != std::string(1, tape->getBlankSymbol())) {
            snapshot.tapeContent[cell.first] = cell.second;
        }
    }

    return snapshot;
}

void TuringMachine::restoreSnapshot(const ExecutionSnapshot& snapshot)
{
    currentState = snapshot.currentState;

    tape->reset();
    for (const auto& pair : snapshot.tapeContent) {
        tape->setHeadPosition(pair.first);
        tape->write(pair.second);
    }

    tape->setHeadPosition(snapshot.headPosition);
}

void TuringMachine::clearHistory()
{
    history.clear();
    historyPosition = -1;
}

void TuringMachine::addToHistory(const ExecutionSnapshot& snapshot)
{
    if (historyPosition >= 0 && historyPosition < static_cast<int>(history.size()) - 1) {
        history.resize(historyPosition + 1);
    }

    history.push_back(snapshot);
    historyPosition = history.size() - 1;

    if (history.size() > static_cast<size_t>(maxHistorySize)) {
        history.erase(history.begin());
        historyPosition--;
    }
}