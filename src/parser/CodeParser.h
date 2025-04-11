#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <regex>

class TuringMachine;
class State;
class Transition;

// Parser for the Turing machine code with special syntax
class CodeParser {
public:
    CodeParser();
    ~CodeParser();
    
    // Parse code and update a Turing machine
    bool parseAndUpdateMachine(TuringMachine* machine, const std::string& code);
    
    // Extract just the states and transitions without updating a machine
    bool parseCode(const std::string& code, 
                  std::vector<std::unique_ptr<State>>& states,
                  std::vector<std::unique_ptr<Transition>>& transitions);

private:
    // State declaration patterns
    std::regex m_startStateRegex;  // s(state_id, [name])
    std::regex m_acceptStateRegex; // a(state_id, [name])
    std::regex m_rejectStateRegex; // r(state_id, [name])
    std::regex m_normalStateRegex; // q(state_id, [name])
    
    // Transition pattern - f(q0, 0) -> (q1, 1, R)
    std::regex m_transitionRegex;
    
    // Parse a single line of code
    bool parseLine(const std::string& line, 
                   std::vector<std::unique_ptr<State>>& states,
                   std::vector<std::unique_ptr<Transition>>& transitions);
    
    // Parse specific line types
    bool parseStateDeclaration(const std::string& line, 
                              std::vector<std::unique_ptr<State>>& states);
    bool parseTransition(const std::string& line, 
                        std::vector<std::unique_ptr<Transition>>& transitions);
                        
    // Helper methods
    std::string trimString(const std::string& str);
    std::vector<std::string> splitString(const std::string& str, char delimiter);
};
