#include "bitboard.h"
#include <algorithm>
#include <stdexcept>
#include <bitset>
#include <random>

// Direction masks for the 8 directions (N, NE, E, SE, S, SW, W, NW)
const std::array<uint64_t, 8> BitBoard::DIRECTION_MASKS = {
    0x0101010101010100ULL, // North
    0x8040201008040200ULL, // Northeast  
    0x00000000000000FEULL, // East
    0x0040201008040201ULL, // Southeast
    0x0001010101010101ULL, // South
    0x0002040810204080ULL, // Southwest
    0x7F00000000000000ULL, // West
    0x8040201008040200ULL  // Northwest
};

// Direction offsets for bit shifting
const std::array<int, 8> BitBoard::DIRECTION_OFFSETS = {
    8,   // North
    7,   // Northeast
    1,   // East
    -9,  // Southeast
    -8,  // South
    -7,  // Southwest
    -1,  // West
    9    // Northwest
};

// Precomputed masks for evaluation
const uint64_t BitBoard::CORNER_MASK = 0x8100000000000081ULL; // Corners: (0,0), (0,7), (7,0), (7,7)
const uint64_t BitBoard::EDGE_MASK = 0x7E8181818181817EULL;   // All edges except corners
const uint64_t BitBoard::STABLE_MASK = 0x0000000000000000ULL; // Will be computed dynamically

// Zobrist hashing tables
std::array<std::array<std::array<uint64_t, 3>, 8>, 8> BitBoard::zobristTable;
bool BitBoard::zobristInitialized = false;

BitBoard::BitBoard() {
    reset();
}

BitBoard::BitBoard(uint64_t black, uint64_t white) 
    : blackBoard(black), whiteBoard(white) {
}

void BitBoard::reset() {
    // Initialize with standard Othello starting position
    blackBoard = 0x0000001008000000ULL; // Black discs at (3,4) and (4,3)
    whiteBoard = 0x0000000810000000ULL; // White discs at (3,3) and (4,4)
}

bool BitBoard::isInBounds(int row, int col) const {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

int BitBoard::positionToBit(int row, int col) {
    return row * BOARD_SIZE + col;
}

std::pair<int, int> BitBoard::bitToPosition(int bit) {
    return {bit / BOARD_SIZE, bit % BOARD_SIZE};
}

uint64_t BitBoard::positionToMask(int row, int col) {
    return 1ULL << positionToBit(row, col);
}

int BitBoard::getCell(int row, int col) const {
    if (!isInBounds(row, col)) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    
    uint64_t mask = positionToMask(row, col);
    if (blackBoard & mask) return 1; // Black
    if (whiteBoard & mask) return 2; // White
    return 0; // Empty
}

void BitBoard::setCell(int row, int col, int state) {
    if (!isInBounds(row, col)) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    
    uint64_t mask = positionToMask(row, col);
    blackBoard &= ~mask;  // Clear black bit
    whiteBoard &= ~mask;  // Clear white bit
    
    if (state == 1) {      // Black
        blackBoard |= mask;
    } else if (state == 2) { // White
        whiteBoard |= mask;
    }
    // state == 0 (empty) leaves both bits cleared
}

uint64_t BitBoard::getPlayerBoard(bool isBlack) const {
    return isBlack ? blackBoard : whiteBoard;
}

uint64_t BitBoard::getOpponentBoard(bool isBlack) const {
    return isBlack ? whiteBoard : blackBoard;
}

uint64_t BitBoard::shift(uint64_t board, int direction) {
    int offset = DIRECTION_OFFSETS[direction];
    if (offset > 0) {
        return board << offset;
    } else {
        return board >> (-offset);
    }
}

uint64_t BitBoard::getLine(int row, int col, int direction) {
    uint64_t line = 0;
    
    // Generate line in the given direction using direction offsets
    int currentRow = row;
    int currentCol = col;
    
    for (int i = 0; i < 8; ++i) {
        if (currentRow >= 0 && currentRow < BOARD_SIZE && currentCol >= 0 && currentCol < BOARD_SIZE) {
            line |= positionToMask(currentRow, currentCol);
        }
        
        // Move in the direction
        switch (direction) {
            case 0: currentRow--; break; // North
            case 1: currentRow--; currentCol++; break; // Northeast
            case 2: currentCol++; break; // East
            case 3: currentRow++; currentCol++; break; // Southeast
            case 4: currentRow++; break; // South
            case 5: currentRow++; currentCol--; break; // Southwest
            case 6: currentCol--; break; // West
            case 7: currentRow--; currentCol--; break; // Northwest
        }
    }
    
    return line;
}

uint64_t BitBoard::getOutflank(uint64_t player, uint64_t opponent, int direction) {
    uint64_t empty = ~(player | opponent);
    uint64_t outflank = 0;
    
    // Check each direction for outflanking moves
    uint64_t temp = shift(player, direction) & opponent;
    while (temp) {
        temp = shift(temp, direction) & opponent;
        outflank |= temp;
    }
    
    return outflank & empty;
}

uint64_t BitBoard::getValidMovesBitboard(bool isBlack) const {
    uint64_t validMoves = 0;
    
    // Check every empty position to see if it's a valid move
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (isValidMove(row, col, isBlack)) {
                validMoves |= positionToMask(row, col);
            }
        }
    }
    
    return validMoves;
}

std::vector<std::pair<int, int>> BitBoard::getValidMoves(bool isBlack) const {
    std::vector<std::pair<int, int>> moves;
    uint64_t validMoves = getValidMovesBitboard(isBlack);
    
    while (validMoves) {
        int bit = __builtin_ctzll(validMoves); // Count trailing zeros
        auto [row, col] = bitToPosition(bit);
        moves.emplace_back(row, col);
        validMoves &= validMoves - 1; // Clear the lowest set bit
    }
    
    return moves;
}

bool BitBoard::isValidMove(int row, int col, bool isBlack) const {
    if (!isInBounds(row, col)) return false;
    
    uint64_t mask = positionToMask(row, col);
    uint64_t occupied = blackBoard | whiteBoard;
    
    if (occupied & mask) return false; // Cell is not empty
    
    // Check if this move would flip any opponent discs
    return getFlippedBitboard(row, col, isBlack) != 0;
}

uint64_t BitBoard::getFlippedBitboard(int row, int col, bool isBlack) const {
    uint64_t flipped = 0;
    uint64_t player = getPlayerBoard(isBlack);
    uint64_t opponent = getOpponentBoard(isBlack);

    for (int dir = 0; dir < 8; ++dir) {
        int dRow = 0, dCol = 0;
        switch (dir) {
            case 0: dRow = -1; break; // N
            case 1: dRow = -1; dCol = 1; break; // NE
            case 2: dCol = 1; break; // E
            case 3: dRow = 1; dCol = 1; break; // SE
            case 4: dRow = 1; break; // S
            case 5: dRow = 1; dCol = -1; break; // SW
            case 6: dCol = -1; break; // W
            case 7: dRow = -1; dCol = -1; break; // NW
        }

        int r = row + dRow;
        int c = col + dCol;
        uint64_t tempFlipped = 0;

        while (isInBounds(r, c)) {
            uint64_t posMask = positionToMask(r, c);
            if (opponent & posMask) {
                tempFlipped |= posMask;
            } else if (player & posMask) {
                flipped |= tempFlipped;
                break;
            } else {
                break;
            }

            r += dRow;
            c += dCol;
        }
    }

    return flipped;
}

std::vector<std::pair<int, int>> BitBoard::getFlippedDiscs(int row, int col, bool isBlack) const {
    std::vector<std::pair<int, int>> flipped;
    uint64_t flippedBitboard = getFlippedBitboard(row, col, isBlack);
    
    while (flippedBitboard) {
        int bit = __builtin_ctzll(flippedBitboard);
        auto [fRow, fCol] = bitToPosition(bit);
        flipped.emplace_back(fRow, fCol);
        flippedBitboard &= flippedBitboard - 1;
    }
    
    return flipped;
}

bool BitBoard::makeMove(int row, int col, bool isBlack) {
    if (!isValidMove(row, col, isBlack)) {
        return false;
    }
    
    uint64_t mask = positionToMask(row, col);
    uint64_t flipped = getFlippedBitboard(row, col, isBlack);
    
    // Place the disc
    if (isBlack) {
        blackBoard |= mask;
    } else {
        whiteBoard |= mask;
    }
    
    // Flip opponent discs
    if (isBlack) {
        blackBoard |= flipped;
        whiteBoard &= ~flipped;
    } else {
        whiteBoard |= flipped;
        blackBoard &= ~flipped;
    }
    
    return true;
}

bool BitBoard::hasValidMoves(bool isBlack) const {
    return getValidMovesBitboard(isBlack) != 0;
}

bool BitBoard::isGameOver() const {
    return !hasValidMoves(true) && !hasValidMoves(false);
}

bool BitBoard::isFull() const {
    return (blackBoard | whiteBoard) == 0xFFFFFFFFFFFFFFFFULL;
}

int BitBoard::getScore(bool isBlack) const {
    return __builtin_popcountll(isBlack ? blackBoard : whiteBoard);
}

int BitBoard::getTotalDiscs() const {
    return __builtin_popcountll(blackBoard | whiteBoard);
}

uint64_t BitBoard::getCornerMask() const {
    return CORNER_MASK;
}

uint64_t BitBoard::getEdgeMask() const {
    return EDGE_MASK;
}

uint64_t BitBoard::getStableMask() const {
    // For now, return empty mask - stability calculation is complex
    // and would require more sophisticated analysis
    return 0;
}

bool BitBoard::operator==(const BitBoard& other) const {
    return blackBoard == other.blackBoard && whiteBoard == other.whiteBoard;
}

bool BitBoard::operator!=(const BitBoard& other) const {
    return !(*this == other);
}

void BitBoard::initializeZobrist() {
    if (zobristInitialized) return;
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    // Generate random keys for each position and player
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            for (int player = 0; player < 3; ++player) { // 0=empty, 1=black, 2=white
                zobristTable[row][col][player] = dis(gen);
            }
        }
    }
    
    zobristInitialized = true;
}

uint64_t BitBoard::getZobristKey(int row, int col, int player) {
    if (!zobristInitialized) {
        initializeZobrist();
    }
    return zobristTable[row][col][player];
}

uint64_t BitBoard::getZobristHash() const {
    if (!zobristInitialized) {
        initializeZobrist();
    }
    
    uint64_t hash = 0;
    
    // XOR all occupied positions
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int cellState = getCell(row, col);
            if (cellState != 0) { // Not empty
                hash ^= zobristTable[row][col][cellState];
            }
        }
    }
    
    return hash;
}
