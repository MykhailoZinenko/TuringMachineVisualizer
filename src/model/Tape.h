#ifndef TAPE_H
#define TAPE_H

#include <map>
#include <vector>
#include <string>

class Tape {
public:
    Tape(char blankSymbol = '_');
    ~Tape();
    
    // Core operations
    char read() const;
    void write(char symbol);
    void moveLeft();
    void moveRight();
    void reset();
    
    // Utility functions
    int getHeadPosition() const;
    void setHeadPosition(int position);
    char getBlankSymbol() const;
    void setInitialContent(const std::string& content);
    std::string getCurrentContent(int windowSize = 20) const;
    
    // For visualization
    std::vector<std::pair<int, char>> getVisiblePortion(int leftOffset, int visibleCells) const;
    int getLeftmostUsedPosition() const;
    int getRightmostUsedPosition() const;

private:
    std::map<int, char> cells;  // Sparse representation
    int headPosition;
    char blankSymbol;
    
    // Track leftmost and rightmost non-blank positions for efficient rendering
    int leftmostUsed;
    int rightmostUsed;
    
    // Update bounds after cell modifications
    void updateBounds(int position);
};

#endif // TAPE_H