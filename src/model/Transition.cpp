#include "Transition.h"
#include <sstream>
#include <vector>

Transition::Transition(const std::string& fromState, char readSymbol,
                       const std::string& toState, char writeSymbol,
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

char Transition::getReadSymbol() const
{
    return readSymbol;
}

void Transition::setReadSymbol(char symbol)
{
    readSymbol = symbol;
}

char Transition::getWriteSymbol() const
{
    return writeSymbol;
}

void Transition::setWriteSymbol(char symbol)
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
    return !fromState.empty() && !toState.empty();
}

std::string Transition::getDisplayText() const
{
    std::string dirText;
    switch (moveDirection) {
        case Direction::LEFT:
            dirText = "L";
            break;
        case Direction::RIGHT:
            dirText = "R";
            break;
        case Direction::STAY:
            dirText = "S";
            break;
    }

    return std::string(1, readSymbol) + " → " +
           std::string(1, writeSymbol) + ", " + dirText;
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
        return Transition("q0", '_', "q1", '_', Direction::RIGHT);
    }

    std::string fromState = parts[0];
    char readSymbol = parts[1].empty() ? '_' : parts[1][0];
    std::string toState = parts[2];
    char writeSymbol = parts[3].empty() ? '_' : parts[3][0];
    Direction direction = static_cast<Direction>(std::stoi(parts[4]));

    return Transition(fromState, readSymbol, toState, writeSymbol, direction);
}