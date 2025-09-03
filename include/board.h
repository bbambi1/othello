#pragma once

#include <vector>
#include <array>
#include <utility>

enum class CellState {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

enum class Direction {
    NORTH = 0,
    NORTHEAST = 1,
    EAST = 2,
    SOUTHEAST = 3,
    SOUTH = 4,
    SOUTHWEST = 5,
    WEST = 6,
    NORTHWEST = 7
};

class Board {
public:
    static constexpr int BOARD_SIZE = 8;
    static constexpr int TOTAL_CELLS = BOARD_SIZE * BOARD_SIZE;

    Board();

    // Game state
    bool isValidMove(int row, int col, CellState player) const;
    std::vector<std::pair<int, int>> getValidMoves(CellState player) const;
    bool makeMove(int row, int col, CellState player);
    bool isGameOver() const;
    bool hasValidMoves(CellState player) const;

    // Board access
    CellState getCell(int row, int col) const;
    void setCell(int row, int col, CellState state);

    // Game statistics
    int getScore(CellState player) const;
    int getTotalDiscs() const;

    // Board state
    bool isFull() const;
    void reset();

    // Utility functions
    bool isInBounds(int row, int col) const;
    std::vector<std::pair<int, int>> getFlippedDiscs(int row, int col, CellState player) const;

private:
    std::array<std::array<CellState, BOARD_SIZE>, BOARD_SIZE> grid;

    // Helper functions
    bool checkDirection(int row, int col, Direction dir, CellState player) const;
    std::vector<std::pair<int, int>> getDiscsInDirection(int row, int col, Direction dir, CellState player) const;
    static std::pair<int, int> getDirectionOffset(Direction dir);
};
