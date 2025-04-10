#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "State.h"
#include "Transition.h"
#include "Tape.h"

enum class MachineType {
    DETERMINISTIC,
    NON_DETERMINISTIC
};

enum class ExecutionStatus {
    READY,
    RUNNING,
    PAUSED,
    HALTED_ACCEPT,
    HALTED_REJECT,
    ERROR
};

struct ExecutionSnapshot {
    std::string currentState;
    int headPosition;
    std::map<int, std::string> tapeContent;  // Changed to store strings

    bool operator==(const ExecutionSnapshot& other) const {
        return currentState == other.currentState &&
               headPosition == other.headPosition &&
               tapeContent == other.tapeContent;
    }

    bool operator!=(const ExecutionSnapshot& other) const {
        return !(*this == other);
    }
};

class TuringMachine {
public:
    // Constructor & destructor
    TuringMachine(const std::string& name = "Untitled Machine",
                  MachineType type = MachineType::DETERMINISTIC);
    ~TuringMachine();

    // Machine configuration
    std::string getName() const;
    void setName(const std::string& name);

    MachineType getType() const;
    void setType(MachineType type);

    // State management
    void addState(const std::string& id, const std::string& name = "", StateType type = StateType::NORMAL);
    void removeState(const std::string& id);
    State* getState(const std::string& id);
    std::vector<State*> getAllStates() const;
    std::string getStartState() const;
    void setStartState(const std::string& id);

    // Transition management - updated to use strings instead of chars
    void addTransition(const std::string& fromState, const std::string& readSymbol,
                       const std::string& toState, const std::string& writeSymbol,
                       Direction moveDirection);
    void removeTransition(const std::string& fromState, const std::string& readSymbol);
    Transition* getTransition(const std::string& fromState, const std::string& readSymbol);
    std::vector<Transition*> getAllTransitions() const;

    // Tape management
    Tape* getTape();
    void setTapeContent(const std::string& content);
    std::string getTapeContent() const;

    // Execution control
    void reset();
    bool step();
    void run();
    void pause();
    bool canStepBackward() const;
    bool stepBackward();
    ExecutionStatus getStatus() const;
    std::string getCurrentState() const;

    // Analysis and statistics
    int getStepCount() const;
    int getMaxHistorySize() const;
    void setMaxHistorySize(int size);

    // Serialization
    std::string toJson() const;
    static std::unique_ptr<TuringMachine> fromJson(const std::string& json);

private:
    std::string name;
    MachineType type;
    std::map<std::string, std::unique_ptr<State>> states;
    std::map<std::pair<std::string, std::string>, std::unique_ptr<Transition>> transitions;  // Changed key to use string
    std::unique_ptr<Tape> tape;

    std::string currentState;
    ExecutionStatus status;
    int stepCount;

    // Execution history
    std::vector<ExecutionSnapshot> history;
    int maxHistorySize;
    int historyPosition;

    // Helper methods
    ExecutionSnapshot createSnapshot() const;
    void restoreSnapshot(const ExecutionSnapshot& snapshot);
    void clearHistory();
    void addToHistory(const ExecutionSnapshot& snapshot);
};