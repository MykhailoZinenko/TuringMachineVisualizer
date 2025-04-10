#include "Transition.h"
#include <sstream>
#include <vector>
#include <regex>
#include <cctype>

Transition::Transition(const std::string& fromState, const std::string& readSymbol,
                       const std::string& toState, const std::string& writeSymbol,
                       Direction moveDirection)
    : fromState(fromState), toState(toState),
      readSymbol(readSymbol), writeSymbol(writeSymbol),
      moveDirection(moveDirection)
{
}

Transition::~Transition()
{
}

std::string Transition::getFromState() const
{
    return fromState;
}

void Transition::setFromState(const std::string& state)
{
    fromState = state;
}

std::string Transition::getToState() const
{
    return toState;
}

void Transition::setToState(const std::string& state)
{
    toState = state;
}

std::string Transition::getReadSymbol() const
{
    return readSymbol;
}

void Transition::setReadSymbol(const std::string& symbol)
{
    readSymbol = symbol;
}

std::string Transition::getWriteSymbol() const
{
    return writeSymbol;
}

void Transition::setWriteSymbol(const std::string& symbol)
{
    writeSymbol = symbol;
}

Direction Transition::getDirection() const
{
    return moveDirection;
}

void Transition::setDirection(Direction direction)
{
    moveDirection = direction;
}

bool Transition::isValid() const
{
    return !fromState.empty() && !toState.empty() && !readSymbol.empty() && !writeSymbol.empty();
}

std::string Transition::getDisplayText() const
{
    std::string dirText = directionToString(moveDirection);
    return readSymbol + " → " + writeSymbol + ", " + dirText;
}

std::string Transition::directionToString(Direction dir)
{
    switch (dir) {
        case Direction::LEFT:
            return "L";
        case Direction::RIGHT:
            return "R";
        case Direction::STAY:
            return "N";
        default:
            return "?";
    }
}

Direction Transition::stringToDirection(const std::string& dirStr)
{
    if (dirStr == "L" || dirStr == "l") {
        return Direction::LEFT;
    } else if (dirStr == "R" || dirStr == "r") {
        return Direction::RIGHT;
    } else if (dirStr == "N" || dirStr == "n" || dirStr == "0" || dirStr == "S" || dirStr == "s") {
        return Direction::STAY;
    }

    // Default to RIGHT if unknown
    return Direction::RIGHT;
}

// Convert transition to f(q1, 0) -> (q1, 0, R) notation
std::string Transition::toFunctionNotation() const
{
    std::string readSym = readSymbol;
    std::string writeSym = writeSymbol;

    // Use "Blank" or "_" for blank symbol
    if (readSym == "_" || readSym.empty()) {
        readSym = "Blank";
    }

    if (writeSym == "_" || writeSym.empty()) {
        writeSym = "Blank";
    }

    std::string dirText = directionToString(moveDirection);

    return "f(" + fromState + ", " + readSym + ") -> (" + toState + ", " + writeSym + ", " + dirText + ")";
}

// Parse transition from f(q1, 0) -> (q1, 0, R) notation
Transition Transition::fromFunctionNotation(const std::string& notation)
{
    // Create regex to match the notation pattern
    // This matches both with and without the 'f' prefix
    // and supports both '->' and '=' as separators
    std::regex pattern(R"((?:f)?\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)\s*(?:->|=)\s*\(\s*([^,]+)\s*,\s*([^,]+)\s*,\s*([^)]+)\s*\))");
    std::smatch matches;

    if (std::regex_search(notation, matches, pattern) && matches.size() == 6) {
        std::string fromState = matches[1].str();
        std::string readSymbol = matches[2].str();
        std::string toState = matches[3].str();
        std::string writeSymbol = matches[4].str();
        std::string dirStr = matches[5].str();

        // Trim whitespace
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\n\r"));
            s.erase(s.find_last_not_of(" \t\n\r") + 1);
        };

        trim(fromState);
        trim(readSymbol);
        trim(toState);
        trim(writeSymbol);
        trim(dirStr);

        // Handle "Blank" keyword
        if (readSymbol == "Blank" || readSymbol == "blank") {
            readSymbol = "_";
        }
        if (writeSymbol == "Blank" || writeSymbol == "blank") {
            writeSymbol = "_";
        }

        // Convert direction string to Direction enum
        Direction dir = stringToDirection(dirStr);

        return Transition(fromState, readSymbol, toState, writeSymbol, dir);
    }

    // Return a default transition if parsing fails
    return Transition("q0", "_", "q0", "_", Direction::RIGHT);
}

std::string Transition::toString() const
{
    return fromState + "|" + readSymbol + "|" +
           toState + "|" + writeSymbol + "|" +
           std::to_string(static_cast<int>(moveDirection));
}

Transition Transition::fromString(const std::string& str)
{
    std::vector<std::string> parts;
    std::string part;
    std::stringstream ss(str);

    while (std::getline(ss, part, '|')) {
        parts.push_back(part);
    }

    if (parts.size() < 5) {
        return Transition("q0", "_", "q1", "_", Direction::RIGHT);
    }

    std::string fromState = parts[0];
    std::string readSymbol = parts[1].empty() ? "_" : parts[1];
    std::string toState = parts[2];
    std::string writeSymbol = parts[3].empty() ? "_" : parts[3];
    Direction direction = static_cast<Direction>(std::stoi(parts[4]));

    return Transition(fromState, readSymbol, toState, writeSymbol, direction);
}