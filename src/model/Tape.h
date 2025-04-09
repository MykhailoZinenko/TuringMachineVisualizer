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
    char read() const;
    void write(char symbol);
    void moveLeft();
    void moveRight();
    void reset();

    // Getters and setters
    int getHeadPosition() const;
    void setHeadPosition(int position);
    char getBlankSymbol() const;

    // Content management
    void setInitialContent(const std::string& content);
    std::string getCurrentContent(int windowSize = 20) const;

    // Visualization support
    std::vector<std::pair<int, char>> getVisiblePortion(int firstCellIndex, int count) const;
    int getLeftmostUsedPosition() const;
    int getRightmostUsedPosition() const;

private:
    std::map<int, char> cells;  // Sparse representation
    int headPosition;
    char blankSymbol;
    int leftmostUsed;
    int rightmostUsed;

    void updateBounds(int position);
};