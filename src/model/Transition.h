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
    Transition(const std::string& fromState, const std::string& readSymbol,
               const std::string& toState, const std::string& writeSymbol,
               Direction moveDirection);
    ~Transition();

    // State accessors
    std::string getFromState() const;
    void setFromState(const std::string& state);

    std::string getToState() const;
    void setToState(const std::string& state);

    // Symbol accessors (now using strings instead of chars)
    std::string getReadSymbol() const;
    void setReadSymbol(const std::string& symbol);

    std::string getWriteSymbol() const;
    void setWriteSymbol(const std::string& symbol);

    // Direction accessors
    Direction getDirection() const;
    void setDirection(Direction direction);

    // Utility methods
    bool isValid() const;
    std::string getDisplayText() const;

    // Format for f(q1, 0) -> (q1, 0, R) notation
    std::string toFunctionNotation() const;
    static Transition fromFunctionNotation(const std::string& notation);

    // Helper for direction conversion
    static std::string directionToString(Direction dir);
    static Direction stringToDirection(const std::string& dirStr);

    // Serialization
    std::string toString() const;
    static Transition fromString(const std::string& str);

private:
    std::string fromState;
    std::string toState;
    std::string readSymbol;   // Changed from char to string
    std::string writeSymbol;  // Changed from char to string
    Direction moveDirection;
};