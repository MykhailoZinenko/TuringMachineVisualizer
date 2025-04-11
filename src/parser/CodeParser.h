#pragma once

#include <string>
#include <vector>
#include <regex>
#include <memory>

class TuringMachine;
class State;
class Transition;

/**
 * Parser for Turing machine code with special syntax
 */
class CodeParser {
public:
    CodeParser();
    ~CodeParser();

    // Parse code and update a machine
    bool parseAndUpdateMachine(TuringMachine* machine, const std::string& code);

private:
    // Regex patterns
    std::regex m_startStateRegex;    // s(state_id, [name])
    std::regex m_acceptStateRegex;   // a(state_id, [name])
    std::regex m_rejectStateRegex;   // r(state_id, [name])
    std::regex m_normalStateRegex;   // q(state_id, [name])
    std::regex m_transitionRegex;    // f(q0, 0) -> (q1, 1, R)

    // Parse state declarations
    bool parseStateDeclaration(const std::string& line,
                              TuringMachine* machine);

    // Parse transitions
    bool parseTransition(const std::string& line,
                        TuringMachine* machine);

    // Helper methods
    std::string trimString(const std::string& str);
};