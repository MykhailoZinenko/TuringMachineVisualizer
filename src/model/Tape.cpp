#include "Tape.h"
#include <algorithm>

Tape::Tape(char blankSymbol)
    : headPosition(0), blankSymbol(blankSymbol),
      leftmostUsed(0), rightmostUsed(0)
{
}

Tape::~Tape()
{
}

char Tape::read() const
{
    auto it = cells.find(headPosition);
    if (it != cells.end()) {
        return it->second;
    }
    return blankSymbol;
}

void Tape::write(char symbol)
{
    if (symbol == blankSymbol) {
        cells.erase(headPosition);
    } else {
        cells[headPosition] = symbol;
    }

    updateBounds(headPosition);
}

void Tape::moveLeft()
{
    headPosition--;
}

void Tape::moveRight()
{
    headPosition++;
}

void Tape::reset()
{
    cells.clear();
    headPosition = 0;
    leftmostUsed = 0;
    rightmostUsed = 0;
}

int Tape::getHeadPosition() const
{
    return headPosition;
}

void Tape::setHeadPosition(int position)
{
    headPosition = position;
}

char Tape::getBlankSymbol() const
{
    return blankSymbol;
}

void Tape::setInitialContent(const std::string& content)
{
    reset();

    for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] != blankSymbol) {
            cells[i] = content[i];
            updateBounds(i);
        }
    }
}

std::string Tape::getCurrentContent(int windowSize) const
{
    int start = headPosition - windowSize / 2;
    int end = headPosition + windowSize / 2;

    std::string result;
    for (int i = start; i <= end; ++i) {
        auto it = cells.find(i);
        if (it != cells.end()) {
            result += it->second;
        } else {
            result += blankSymbol;
        }
    }

    return result;
}

std::vector<std::pair<int, char>> Tape::getVisiblePortion(int firstCellIndex, int count) const
{
    std::vector<std::pair<int, char>> result;

    auto it = cells.find(firstCellIndex);
    if (it != cells.end()) {
        result.push_back(std::make_pair(firstCellIndex, it->second));
    } else {
        result.push_back(std::make_pair(firstCellIndex, blankSymbol));
    }

    return result;
}

int Tape::getLeftmostUsedPosition() const
{
    return leftmostUsed;
}

int Tape::getRightmostUsedPosition() const
{
    return rightmostUsed;
}

void Tape::updateBounds(int position)
{
    auto it = cells.find(position);
    if (it == cells.end()) {
        return;
    }

    leftmostUsed = std::min(leftmostUsed, position);
    rightmostUsed = std::max(rightmostUsed, position);
}