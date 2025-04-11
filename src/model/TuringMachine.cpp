#include "TuringMachine.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <QtCore/qstring.h>
#include <qdebug.h>

using json = nlohmann::json;

// Constructor & destructor
TuringMachine::TuringMachine(const std::string& name, MachineType type)
    : name(name), type(type), activeTape(nullptr), status(ExecutionStatus::READY),
      stepCount(0), maxHistorySize(1000), historyPosition(-1)
{
}

TuringMachine::~TuringMachine()
{
    // We don't own activeTape, so we don't delete it
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

// Transition management
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

// Tape operations
void TuringMachine::setTape(Tape* tape)
{
    activeTape = tape;
}

// Code management
void TuringMachine::setOriginalCode(const std::string& code)
{
    m_originalCode = code;
}

std::string TuringMachine::getOriginalCode() const
{
    return m_originalCode;
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

    if (activeTape) {
        activeTape->reset();
    }

    status = ExecutionStatus::READY;
    stepCount = 0;
    clearHistory();

    addToHistory(createSnapshot());
}

bool TuringMachine::step()
{
    if (!activeTape) {
        qWarning() << "Cannot step: no active tape set";
        return false;
    }

    if (status == ExecutionStatus::HALTED_ACCEPT ||
        status == ExecutionStatus::HALTED_REJECT ||
        status == ExecutionStatus::ERROR) {
        return false;
    }

    // Temporarily set to RUNNING
    ExecutionStatus oldStatus = status;
    status = ExecutionStatus::RUNNING;

    State* state = getState(currentState);
    if (!state) {
        status = ExecutionStatus::ERROR;
        qWarning() << "Error: No valid state" << QString::fromStdString(currentState);
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

    std::string symbol = activeTape->read();
    Transition* transition = getTransition(currentState, symbol);

    if (!transition) {
        // Try with the blank symbol as a fallback
        transition = getTransition(currentState, std::string(1, activeTape->getBlankSymbol()));
    }

    if (!transition) {
        status = ExecutionStatus::ERROR;
        qWarning() << "Error: No transition found for state" << QString::fromStdString(currentState)
                 << "and symbol" << QString::fromStdString(symbol);
        return false;
    }

    // Execute the transition
    activeTape->write(transition->getWriteSymbol());

    switch (transition->getDirection()) {
        case Direction::LEFT:
            activeTape->moveLeft();
            break;
        case Direction::RIGHT:
            activeTape->moveRight();
            break;
        case Direction::STAY:
            break;
    }

    currentState = transition->getToState();

    stepCount++;
    addToHistory(createSnapshot());

    // Set back to PAUSED or original state after a single step
    if (oldStatus == ExecutionStatus::PAUSED || oldStatus == ExecutionStatus::READY) {
        status = oldStatus;
    } else {
        status = ExecutionStatus::PAUSED;
    }

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
    if (!activeTape) {
        qWarning() << "Cannot step backward: no active tape set";
        return false;
    }

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
    j["originalCode"] = m_originalCode;

    // Save states
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

    // Save transitions
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
    try {
        json j = json::parse(jsonStr);

        auto machine = std::make_unique<TuringMachine>(j.value("name", "Untitled"),
                                                      static_cast<MachineType>(j.value("type", 0)));

        // Load original code if present
        if (j.contains("originalCode")) {
            machine->setOriginalCode(j["originalCode"]);
        }

        // Load states
        if (j.contains("states") && j["states"].is_array()) {
            for (const auto& stateJson : j["states"]) {
                try {
                    std::string id = stateJson["id"];
                    std::string name = stateJson["name"];
                    StateType type = static_cast<StateType>(stateJson["type"].get<int>());

                    machine->addState(id, name, type);

                    State* state = machine->getState(id);
                    if (state && stateJson.contains("posX") && stateJson.contains("posY")) {
                        state->setPosition(Point2D(stateJson["posX"], stateJson["posY"]));
                    }
                } catch (const std::exception& e) {
                    qWarning() << "Error loading state:" << e.what();
                }
            }
        }

        // Load transitions
        if (j.contains("transitions") && j["transitions"].is_array()) {
            for (const auto& transJson : j["transitions"]) {
                try {
                    std::string fromState = transJson["fromState"];
                    std::string readSymbol;

                    // Handle readSymbol
                    if (transJson["readSymbol"].is_string()) {
                        readSymbol = transJson["readSymbol"];
                    } else if (transJson["readSymbol"].is_number()) {
                        // For backwards compatibility with older files
                        char readChar = static_cast<char>(transJson["readSymbol"].get<int>());
                        readSymbol = std::string(1, readChar);
                    } else {
                        readSymbol = "_";
                    }

                    std::string toState = transJson["toState"];
                    std::string writeSymbol;

                    // Handle writeSymbol
                    if (transJson["writeSymbol"].is_string()) {
                        writeSymbol = transJson["writeSymbol"];
                    } else if (transJson["writeSymbol"].is_number()) {
                        // For backwards compatibility
                        char writeChar = static_cast<char>(transJson["writeSymbol"].get<int>());
                        writeSymbol = std::string(1, writeChar);
                    } else {
                        writeSymbol = "_";
                    }

                    Direction direction = static_cast<Direction>(transJson["direction"].get<int>());

                    machine->addTransition(
                        fromState,
                        readSymbol,
                        toState,
                        writeSymbol,
                        direction
                    );
                } catch (const std::exception& e) {
                    qWarning() << "Error loading transition:" << e.what();
                }
            }
        }

        // Set current state
        if (j.contains("currentState")) {
            machine->currentState = j["currentState"];
        } else {
            machine->currentState = machine->getStartState();
        }

        return machine;
    } catch (const json::exception& e) {
        qCritical() << "JSON parsing error:" << e.what();
        throw;
    } catch (const std::exception& e) {
        qCritical() << "Error in fromJson:" << e.what();
        throw;
    }
}

// Helper methods
ExecutionSnapshot TuringMachine::createSnapshot() const
{
    if (!activeTape) {
        // Return an empty snapshot if no tape is set
        return ExecutionSnapshot{};
    }

    ExecutionSnapshot snapshot;
    snapshot.currentState = currentState;
    snapshot.headPosition = activeTape->getHeadPosition();

    // Get the tape content
    int left = activeTape->getLeftmostUsedPosition();
    int right = activeTape->getRightmostUsedPosition();

    auto visibleCells = activeTape->getVisiblePortion(left, right - left + 1);
    for (const auto& cell : visibleCells) {
        if (cell.second != std::string(1, activeTape->getBlankSymbol())) {
            snapshot.tapeContent[cell.first] = cell.second;
        }
    }

    return snapshot;
}

void TuringMachine::restoreSnapshot(const ExecutionSnapshot& snapshot)
{
    if (!activeTape) {
        return;
    }

    currentState = snapshot.currentState;

    activeTape->reset();

    for (const auto& pair : snapshot.tapeContent) {
        activeTape->setHeadPosition(pair.first);
        activeTape->write(pair.second);
    }

    activeTape->setHeadPosition(snapshot.headPosition);
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