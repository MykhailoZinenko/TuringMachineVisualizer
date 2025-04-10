#pragma once

#include <map>
#include <string>
#include <vector>

class Tape {
public:
    // Constructor & destructor
    Tape(char blankSymbol = '_');
    ~Tape();

    // Core operations
    std::string read() const;  // Changed to return string instead of char
    void write(const std::string& symbols);  // Changed to accept string instead of char
    void moveLeft();
    void moveRight();
    void reset();

    // Getters and setters
    int getHeadPosition() const;
    void setHeadPosition(int position);
    char getBlankSymbol() const;
    std::string getBlankSymbolAsString() const;

    // Content management
    void setInitialContent(const std::string& content);
    std::string getCurrentContent(int windowSize = 20) const;

    // Visualization support
    std::vector<std::pair<int, std::string>> getVisiblePortion(int firstCellIndex, int count) const;
    int getLeftmostUsedPosition() const;
    int getRightmostUsedPosition() const;

private:
    std::map<int, std::string> cells;  // Changed from char to string
    int headPosition;
    char blankSymbol;
    int leftmostUsed;
    int rightmostUsed;

    void updateBounds(int position);
};