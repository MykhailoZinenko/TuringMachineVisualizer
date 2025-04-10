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

std::string Tape::read() const
{
    auto it = cells.find(headPosition);
    if (it != cells.end()) {
        return it->second;
    }
    return std::string(1, blankSymbol);
}

void Tape::write(const std::string& symbols)
{
    if (symbols.empty() || symbols == std::string(1, blankSymbol)) {
        cells.erase(headPosition);
    } else {
        cells[headPosition] = symbols;
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

std::string Tape::getBlankSymbolAsString() const
{
    return std::string(1, blankSymbol);
}

void Tape::setInitialContent(const std::string& content)
{
    reset();

    for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] != blankSymbol) {
            cells[i] = std::string(1, content[i]);
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

std::vector<std::pair<int, std::string>> Tape::getVisiblePortion(int firstCellIndex, int count) const
{
    std::vector<std::pair<int, std::string>> result;

    for (int i = 0; i < count; ++i) {
        int cellIndex = firstCellIndex + i;
        auto it = cells.find(cellIndex);
        if (it != cells.end()) {
            result.push_back(std::make_pair(cellIndex, it->second));
        } else {
            result.push_back(std::make_pair(cellIndex, std::string(1, blankSymbol)));
        }
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