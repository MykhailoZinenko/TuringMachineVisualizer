#include "CodeParser.h"
#include "../model/TuringMachine.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <set>

CodeParser::CodeParser() {
    // Initialize regex patterns
    
    // State patterns
    // s(q0, Start State) - Start state
    m_startStateRegex = std::regex(R"(s\s*\(\s*([a-zA-Z0-9_]+)\s*(?:,\s*([^)]*))?\s*\))");
    
    // a(q1, Accept State) - Accept state
    m_acceptStateRegex = std::regex(R"(a\s*\(\s*([a-zA-Z0-9_]+)\s*(?:,\s*([^)]*))?\s*\))");
    
    // r(q2, Reject State) - Reject state
    m_rejectStateRegex = std::regex(R"(r\s*\(\s*([a-zA-Z0-9_]+)\s*(?:,\s*([^)]*))?\s*\))");
    
    // q(q3, Normal State) - Normal state (optional)
    m_normalStateRegex = std::regex(R"(q\s*\(\s*([a-zA-Z0-9_]+)\s*(?:,\s*([^)]*))?\s*\))");
    
    // Transition patterns
    // f(q0, 0) -> (q1, 1, R) or f(q0, 0) = (q1, 1, R)
    m_transitionRegex = std::regex(
        R"(f\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([^)]*)\s*\)\s*(?:->|=)\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([^,]*)\s*,\s*([LRN])\s*\))"
    );
}

CodeParser::~CodeParser() {
}

bool CodeParser::parseAndUpdateMachine(TuringMachine* machine, const std::string& code) {
    if (!machine) return false;
    
    // Parse the code into states and transitions
    std::vector<std::unique_ptr<State>> states;
    std::vector<std::unique_ptr<Transition>> transitions;
    
    if (!parseCode(code, states, transitions)) {
        return false;
    }
    
    // First, clear existing states and transitions from the machine
    auto existingStates = machine->getAllStates();
    for (auto* state : existingStates) {
        machine->removeState(state->getId());
    }
    
    // Add all states to the machine
    std::string startStateId;
    for (const auto& state : states) {
        std::string stateId = state->getId();
        std::string stateName = state->getName();
        StateType stateType = state->getType();
        
        machine->addState(stateId, stateName, stateType);
        
        if (stateType == StateType::START) {
            startStateId = stateId;
        }
    }
    
    // Set the start state
    if (!startStateId.empty()) {
        machine->setStartState(startStateId);
    } else if (!states.empty()) {
        // If no explicit start state, use the first state
        machine->setStartState(states[0]->getId());
    }
    
    // Add all transitions
    for (const auto& transition : transitions) {
        machine->addTransition(
            transition->getFromState(),
            transition->getReadSymbol(),
            transition->getToState(),
            transition->getWriteSymbol(),
            transition->getDirection()
        );
    }
    
    return true;
}

bool CodeParser::parseCode(const std::string& code, 
                          std::vector<std::unique_ptr<State>>& states,
                          std::vector<std::unique_ptr<Transition>>& transitions) {
    // Clear output vectors
    states.clear();
    transitions.clear();
    
    // Process each line
    std::istringstream stream(code);
    std::string line;
    std::set<std::string> uniqueStates;

    while (std::getline(stream, line)) {
        // Skip empty lines and comments
        line = trimString(line);
        if (line.empty() || line.substr(0, 2) == "//") {
            continue;
        }

        // Parse the line
        if (!parseLine(line, states, transitions)) {
            // Continue even if a line fails, to collect as many valid elements as possible
            std::cerr << "Warning: Failed to parse line: " << line << std::endl;
        }
    }

    // If no states were explicitly declared, extract from transitions
    if (states.empty() && !transitions.empty()) {
        // Collect all unique states from transitions
        std::set<std::string> fromStates, toStates;
        for (const auto& transition : transitions) {
            fromStates.insert(transition->getFromState());
            toStates.insert(transition->getToState());
        }

        // Combine all unique states
        uniqueStates.insert(fromStates.begin(), fromStates.end());
        uniqueStates.insert(toStates.begin(), toStates.end());

        // Create states with default naming
        std::vector<std::string> stateList(uniqueStates.begin(), uniqueStates.end());

        // Ensure we have at least one state before accessing
        if (!stateList.empty()) {
            // Default start state (first state)
            states.push_back(std::make_unique<State>(stateList[0], stateList[0], StateType::START));

            // Default accept state (last state)
            if (stateList.size() > 1) {
                states.push_back(std::make_unique<State>(stateList.back(), stateList.back(), StateType::ACCEPT));
            }

            // Add remaining states as normal states
            for (size_t i = 1; i < stateList.size() - 1; ++i) {
                states.push_back(std::make_unique<State>(stateList[i], stateList[i], StateType::NORMAL));
            }
        }
    }
    
    return true;
}

bool CodeParser::parseLine(const std::string& line, 
                           std::vector<std::unique_ptr<State>>& states,
                           std::vector<std::unique_ptr<Transition>>& transitions) {
    // Try to parse as a state declaration
    if (parseStateDeclaration(line, states)) {
        return true;
    }
    
    // Try to parse as a transition
    if (parseTransition(line, transitions)) {
        return true;
    }
    
    // Line didn't match any pattern
    return false;
}

bool CodeParser::parseStateDeclaration(const std::string& line, 
                                     std::vector<std::unique_ptr<State>>& states) {
    std::smatch matches;
    StateType stateType = StateType::NORMAL;
    
    // Try to match against the different state patterns
    if (std::regex_match(line, matches, m_startStateRegex)) {
        stateType = StateType::START;
    } else if (std::regex_match(line, matches, m_acceptStateRegex)) {
        stateType = StateType::ACCEPT;
    } else if (std::regex_match(line, matches, m_rejectStateRegex)) {
        stateType = StateType::REJECT;
    } else if (std::regex_match(line, matches, m_normalStateRegex)) {
        stateType = StateType::NORMAL;
    } else {
        // Not a state declaration
        return false;
    }
    
    // Extract state ID and name
    std::string stateId = matches[1].str();
    std::string stateName = matches.size() > 2 ? trimString(matches[2].str()) : "";
    
    // Create the state
    auto state = std::make_unique<State>(stateId, stateName, stateType);
    
    // Check for duplicate state IDs
    for (const auto& existingState : states) {
        if (existingState->getId() == stateId) {
            // Update existing state instead of adding a new one
            existingState->setName(stateName);
            existingState->setType(stateType);
            return true;
        }
    }
    
    // Add to the states list
    states.push_back(std::move(state));
    return true;
}

bool CodeParser::parseTransition(const std::string& line,
                                std::vector<std::unique_ptr<Transition>>& transitions) {
    std::smatch matches;

    if (std::regex_match(line, matches, m_transitionRegex)) {
        // Extract transition components
        std::string fromState = trimString(matches[1].str());
        std::string readSymbol = trimString(matches[2].str());
        std::string toState = trimString(matches[3].str());
        std::string writeSymbol = trimString(matches[4].str());

        // Handle special "Blank" keyword
        if (readSymbol == "Blank" || readSymbol == "blank") {
            readSymbol = "_";
        }
        if (writeSymbol == "Blank" || writeSymbol == "blank") {
            writeSymbol = "_";
        }

        // Parse direction
        std::string dirStr = trimString(matches[5].str());
        Direction direction;
        if (dirStr == "L") {
            direction = Direction::LEFT;
        } else if (dirStr == "R") {
            direction = Direction::RIGHT;
        } else if (dirStr == "N") {
            direction = Direction::STAY;
        } else {
            // Invalid direction
            return false;
        }

        // Create the transition
        auto transition = std::make_unique<Transition>(
            fromState,
            readSymbol,
            toState,
            writeSymbol,
            direction
        );

        // Add to the transitions list
        transitions.push_back(std::move(transition));
        return true;
    }

    // Not a transition
    return false;
}

std::string CodeParser::trimString(const std::string& str) {
    // Find first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return ""; // String is all whitespace
    }

    // Find last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n\r");

    // Return the trimmed string
    return str.substr(start, end - start + 1);
}

std::vector<std::string> CodeParser::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trimString(token));
    }

    return tokens;
}