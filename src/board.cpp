#include "board.h"
#include <algorithm>
#include <stdexcept>

Board::Board() {
    reset();
}

void Board::reset() {
    // Initialize all cells as empty
    for (auto& row : grid) {
        std::fill(row.begin(), row.end(), CellState::EMPTY);
    }
    
    // Set up initial 4 discs in the center
    int center = BOARD_SIZE / 2;
    grid[center - 1][center - 1] = CellState::WHITE;
    grid[center - 1][center] = CellState::BLACK;
    grid[center][center - 1] = CellState::BLACK;
    grid[center][center] = CellState::WHITE;
}

bool Board::isInBounds(int row, int col) const {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

CellState Board::getCell(int row, int col) const {
    if (!isInBounds(row, col)) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    return grid[row][col];
}

void Board::setCell(int row, int col, CellState state) {
    if (!isInBounds(row, col)) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    grid[row][col] = state;
}

std::pair<int, int> Board::getDirectionOffset(Direction dir) {
    switch (dir) {
        case Direction::NORTH: return {-1, 0};
        case Direction::NORTHEAST: return {-1, 1};
        case Direction::EAST: return {0, 1};
        case Direction::SOUTHEAST: return {1, 1};
        case Direction::SOUTH: return {1, 0};
        case Direction::SOUTHWEST: return {1, -1};
        case Direction::WEST: return {0, -1};
        case Direction::NORTHWEST: return {-1, -1};
        default: return {0, 0};
    }
}

bool Board::checkDirection(int row, int col, Direction dir, CellState player) const {
    auto [dRow, dCol] = getDirectionOffset(dir);
    int currentRow = row + dRow;
    int currentCol = col + dCol;
    
    // Must have at least one opponent disc in this direction
    if (!isInBounds(currentRow, currentCol) || 
        grid[currentRow][currentCol] == player || 
        grid[currentRow][currentCol] == CellState::EMPTY) {
        return false;
    }
    
    // Look for a disc of the same color
    currentRow += dRow;
    currentCol += dCol;
    
    while (isInBounds(currentRow, currentCol)) {
        if (grid[currentRow][currentCol] == CellState::EMPTY) {
            return false;
        }
        if (grid[currentRow][currentCol] == player) {
            return true;
        }
        currentRow += dRow;
        currentCol += dCol;
    }
    
    return false;
}

std::vector<std::pair<int, int>> Board::getDiscsInDirection(int row, int col, Direction dir, CellState player) const {
    std::vector<std::pair<int, int>> discs;
    auto [dRow, dCol] = getDirectionOffset(dir);
    int currentRow = row + dRow;
    int currentCol = col + dCol;
    
    while (isInBounds(currentRow, currentCol) && grid[currentRow][currentCol] != player) {
        if (grid[currentRow][currentCol] == CellState::EMPTY) {
            break;
        }
        discs.emplace_back(currentRow, currentCol);
        currentRow += dRow;
        currentCol += dCol;
    }
    
    return discs;
}

bool Board::isValidMove(int row, int col, CellState player) const {
    // Check if cell is empty
    if (!isInBounds(row, col) || grid[row][col] != CellState::EMPTY) {
        return false;
    }
    
    // Check all 8 directions
    for (int dir = 0; dir < 8; ++dir) {
        if (checkDirection(row, col, static_cast<Direction>(dir), player)) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::pair<int, int>> Board::getValidMoves(CellState player) const {
    std::vector<std::pair<int, int>> moves;
    
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (isValidMove(row, col, player)) {
                moves.emplace_back(row, col);
            }
        }
    }
    
    return moves;
}

std::vector<std::pair<int, int>> Board::getFlippedDiscs(int row, int col, CellState player) const {
    std::vector<std::pair<int, int>> flipped;
    
    for (int dir = 0; dir < 8; ++dir) {
        if (checkDirection(row, col, static_cast<Direction>(dir), player)) {
            auto discs = getDiscsInDirection(row, col, static_cast<Direction>(dir), player);
            flipped.insert(flipped.end(), discs.begin(), discs.end());
        }
    }
    
    return flipped;
}

bool Board::makeMove(int row, int col, CellState player) {
    if (!isValidMove(row, col, player)) {
        return false;
    }
    
    // Place the disc
    grid[row][col] = player;
    
    // Flip opponent discs
    auto flipped = getFlippedDiscs(row, col, player);
    for (const auto& [fRow, fCol] : flipped) {
        grid[fRow][fCol] = player;
    }
    
    return true;
}

bool Board::hasValidMoves(CellState player) const {
    return !getValidMoves(player).empty();
}

bool Board::isGameOver() const {
    return !hasValidMoves(CellState::BLACK) && !hasValidMoves(CellState::WHITE);
}

bool Board::isFull() const {
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell == CellState::EMPTY) {
                return false;
            }
        }
    }
    return true;
}

int Board::getScore(CellState player) const {
    int score = 0;
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            if (cell == player) {
                score++;
            }
        }
    }
    return score;
}

int Board::getTotalDiscs() const {
    return getScore(CellState::BLACK) + getScore(CellState::WHITE);
}
