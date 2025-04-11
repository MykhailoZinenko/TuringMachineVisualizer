#include "CodeParser.h"
#include "../model/TuringMachine.h"
#include "../model/State.h"
#include "../model/Transition.h"
#include <QDebug>
#include <sstream>

CodeParser::CodeParser()
{
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

CodeParser::~CodeParser()
{
}

bool CodeParser::parseAndUpdateMachine(TuringMachine* machine, const std::string& code)
{
    if (!machine) {
        qWarning() << "Null machine provided to parser";
        return false;
    }

    // Clear existing states and transitions
    auto existingStates = machine->getAllStates();
    for (auto state : existingStates) {
        machine->removeState(state->getId());
    }

    // Process each line
    std::istringstream stream(code);
    std::string line;
    bool foundStates = false;
    bool foundTransitions = false;

    while (std::getline(stream, line)) {
        // Skip empty lines and comments
        line = trimString(line);
        if (line.empty() || line.substr(0, 2) == "//") {
            continue;
        }

        // Try to parse as a state
        if (parseStateDeclaration(line, machine)) {
            foundStates = true;
            continue;
        }

        // Try to parse as a transition
        if (parseTransition(line, machine)) {
            foundTransitions = true;
            continue;
        }
    }

    // If we have transitions but no states, create a default start state
    if (foundTransitions && !foundStates) {
        machine->addState("q0", "Start State", StateType::START);
    }

    return true;
}

bool CodeParser::parseStateDeclaration(const std::string& line, TuringMachine* machine)
{
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

    // Add or update the state in the machine
    State* existingState = machine->getState(stateId);
    if (existingState) {
        existingState->setName(stateName);
        existingState->setType(stateType);
    } else {
        machine->addState(stateId, stateName, stateType);
    }

    // If this is a start state, ensure it's set as the machine's start state
    if (stateType == StateType::START) {
        machine->setStartState(stateId);
    }

    return true;
}

bool CodeParser::parseTransition(const std::string& line, TuringMachine* machine)
{
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

        // Create states if they don't exist
        if (!machine->getState(fromState)) {
            machine->addState(fromState);
        }

        if (!machine->getState(toState)) {
            machine->addState(toState);
        }

        // Add or update the transition
        machine->addTransition(fromState, readSymbol, toState, writeSymbol, direction);

        return true;
    }

    // Not a transition
    return false;
}

std::string CodeParser::trimString(const std::string& str)
{
    // Find first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";  // String is all whitespace
    }

    // Find last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n\r");

    // Return the trimmed string
    return str.substr(start, end - start + 1);
}