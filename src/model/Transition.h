#pragma once

#include <string>

enum class Direction {
    LEFT,
    RIGHT,
    STAY
};

class Transition {
public:
    // Constructor & destructor
    Transition(const std::string& fromState, char readSymbol,
               const std::string& toState, char writeSymbol,
               Direction moveDirection);
    ~Transition();

    // State accessors
    std::string getFromState() const;
    void setFromState(const std::string& state);

    std::string getToState() const;
    void setToState(const std::string& state);

    // Symbol accessors
    char getReadSymbol() const;
    void setReadSymbol(char symbol);

    char getWriteSymbol() const;
    void setWriteSymbol(char symbol);

    // Direction accessors
    Direction getDirection() const;
    void setDirection(Direction direction);

    // Utility methods
    bool isValid() const;
    std::string getDisplayText() const;

    // Serialization
    std::string toString() const;
    static Transition fromString(const std::string& str);

private:
    std::string fromState;
    std::string toState;
    char readSymbol;
    char writeSymbol;
    Direction moveDirection;
};