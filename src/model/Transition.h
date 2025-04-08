#ifndef TRANSITION_H
#define TRANSITION_H

#include <string>

enum class Direction {
    LEFT,
    RIGHT,
    STAY
};

class Transition {
public:
    Transition(const std::string& fromState, char readSymbol, 
               const std::string& toState, char writeSymbol, 
               Direction moveDirection);
    ~Transition();
    
    // Getters and setters
    std::string getFromState() const;
    void setFromState(const std::string& state);
    
    std::string getToState() const;
    void setToState(const std::string& state);
    
    char getReadSymbol() const;
    void setReadSymbol(char symbol);
    
    char getWriteSymbol() const;
    void setWriteSymbol(char symbol);
    
    Direction getDirection() const;
    void setDirection(Direction direction);
    
    // For serialization
    std::string toString() const;
    static Transition fromString(const std::string& str);
    
    // For validation and display
    bool isValid() const;
    std::string getDisplayText() const;

private:
    std::string fromState;
    std::string toState;
    char readSymbol;
    char writeSymbol;
    Direction moveDirection;
};

#endif // TRANSITION_H