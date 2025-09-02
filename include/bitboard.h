#pragma once

#include <cstdint>
#include <vector>
#include <array>

// Bitboard representation for Othello
// Uses 64-bit integers to represent board positions
class BitBoard {
public:
    static constexpr int BOARD_SIZE = 8;
    static constexpr int TOTAL_CELLS = BOARD_SIZE * BOARD_SIZE;
    
    // Bitboard representation: 0 = empty, 1 = occupied
    // blackBoard: 1 = black disc, 0 = not black
    // whiteBoard: 1 = white disc, 0 = not white
    uint64_t blackBoard;
    uint64_t whiteBoard;
    
    BitBoard();
    BitBoard(uint64_t black, uint64_t white);
    
    // Copy constructor and assignment
    BitBoard(const BitBoard& other) = default;
    BitBoard& operator=(const BitBoard& other) = default;
    
    // Game state operations
    bool isValidMove(int row, int col, bool isBlack) const;
    std::vector<std::pair<int, int>> getValidMoves(bool isBlack) const;
    bool makeMove(int row, int col, bool isBlack);
    bool isGameOver() const;
    bool hasValidMoves(bool isBlack) const;
    
    // Board access
    int getCell(int row, int col) const; // 0=empty, 1=black, 2=white
    void setCell(int row, int col, int state);
    
    // Game statistics
    int getScore(bool isBlack) const;
    int getTotalDiscs() const;
    
    // Board state
    bool isFull() const;
    void reset();
    
    // Utility functions
    bool isInBounds(int row, int col) const;
    std::vector<std::pair<int, int>> getFlippedDiscs(int row, int col, bool isBlack) const;
    
    // Bitboard-specific operations
    uint64_t getValidMovesBitboard(bool isBlack) const;
    uint64_t getFlippedBitboard(int row, int col, bool isBlack) const;
    
    // Evaluation helpers
    uint64_t getCornerMask() const;
    uint64_t getEdgeMask() const;
    uint64_t getStableMask() const;
    
    // Conversion utilities
    static int positionToBit(int row, int col);
    static std::pair<int, int> bitToPosition(int bit);
    static uint64_t positionToMask(int row, int col);
    
    // Comparison operators
    bool operator==(const BitBoard& other) const;
    bool operator!=(const BitBoard& other) const;
    
    // Helper functions for AI agents
    uint64_t getOpponentBoard(bool isBlack) const;
    uint64_t getPlayerBoard(bool isBlack) const;
    
    // Zobrist hashing
    uint64_t getZobristHash() const;
    static void initializeZobrist();
    static uint64_t getZobristKey(int row, int col, int player);
    
private:
    // Direction masks for move generation
    static const std::array<uint64_t, 8> DIRECTION_MASKS;
    static const std::array<int, 8> DIRECTION_OFFSETS;
    
    // Precomputed masks for evaluation
    static const uint64_t CORNER_MASK;
    static const uint64_t EDGE_MASK;
    static const uint64_t STABLE_MASK;
    
    // Zobrist hashing tables
    static std::array<std::array<std::array<uint64_t, 3>, 8>, 8> zobristTable;
    static bool zobristInitialized;
    bool checkDirection(int row, int col, int direction, bool isBlack) const;
    uint64_t getDiscsInDirection(int row, int col, int direction, bool isBlack) const;
    
    // Bit manipulation helpers
    static uint64_t shift(uint64_t board, int direction);
    static uint64_t getLine(int row, int col, int direction);
    static uint64_t getOutflank(uint64_t player, uint64_t opponent, int direction);
};
