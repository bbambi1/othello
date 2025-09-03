#include "bitboard.h"
#include <algorithm>
#include <bitset>

std::array<std::array<std::array<uint64_t,3>,8>,8> BitBoard::zobristTable;
bool BitBoard::zobristInitialised = false;

BitBoard::BitBoard() {
    reset();
}

BitBoard::BitBoard(uint64_t black, uint64_t white)
    : blackBoard(black), whiteBoard(white) {
}

void BitBoard::reset() {
    blackBoard = 0;
    whiteBoard = 0;
    // Starting position: black at d5 (3,4) and e4 (4,3), white at d4 (3,3) and e5 (4,4)
    // Note: rows/cols are zero‑indexed.  The positions are symmetrical around the centre.
    setCell(3, 4, 1);
    setCell(4, 3, 1);
    setCell(3, 3, 2);
    setCell(4, 4, 2);
}

int BitBoard::positionToBit(int row, int col) {
    return row * BOARD_SIZE + col;
}

std::pair<int,int> BitBoard::bitToPosition(int bit) {
    return {bit / BOARD_SIZE, bit % BOARD_SIZE};
}

uint64_t BitBoard::positionToMask(int row, int col) {
    return 1ULL << positionToBit(row, col);
}

int BitBoard::getCell(int row, int col) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    uint64_t mask = positionToMask(row, col);
    if (blackBoard & mask) return 1;
    if (whiteBoard & mask) return 2;
    return 0;
}

void BitBoard::setCell(int row, int col, int state) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        throw std::out_of_range("Cell coordinates out of bounds");
    }
    uint64_t mask = positionToMask(row, col);
    // Clear both bits
    blackBoard &= ~mask;
    whiteBoard &= ~mask;
    if (state == 1) {
        blackBoard |= mask;
    } else if (state == 2) {
        whiteBoard |= mask;
    }
}

uint64_t BitBoard::getPlayerBoard(bool isBlack) const {
    return isBlack ? blackBoard : whiteBoard;
}

// Corner mask includes the four corner positions
uint64_t BitBoard::getCornerMask() const {
    // Precompute once
    static const uint64_t cornerMask = []() {
        uint64_t m = 0;
        // Corners: (0,0), (0,7), (7,0), (7,7)
        m |= positionToMask(0, 0);
        m |= positionToMask(0, 7);
        m |= positionToMask(7, 0);
        m |= positionToMask(7, 7);
        return m;
    }();
    return cornerMask;
}

// Edge mask comprises all squares on the outer edge except the four corners
uint64_t BitBoard::getEdgeMask() const {
    static const uint64_t edgeMask = []() {
        uint64_t m = 0;
        // Top row and bottom row
        for (int c = 0; c < BOARD_SIZE; ++c) {
            m |= positionToMask(0, c);
            m |= positionToMask(BOARD_SIZE - 1, c);
        }
        // Left and right columns (exclude corners already added)
        for (int r = 1; r < BOARD_SIZE - 1; ++r) {
            m |= positionToMask(r, 0);
            m |= positionToMask(r, BOARD_SIZE - 1);
        }
        // Remove corners from this mask to keep them distinct
        uint64_t corners = positionToMask(0, 0) | positionToMask(0, 7) |
                           positionToMask(BOARD_SIZE - 1, 0) |
                           positionToMask(BOARD_SIZE - 1, BOARD_SIZE - 1);
        return m & ~corners;
    }();
    return edgeMask;
}

// Stability mask placeholder: returns zero for now.  A full
// implementation would analyse which discs cannot be flipped.
uint64_t BitBoard::getStableMask() const {
    return 0ULL;
}

uint64_t BitBoard::getOpponentBoard(bool isBlack) const {
    return isBlack ? whiteBoard : blackBoard;
}

bool BitBoard::isValidMove(int row, int col, bool isBlack) const {
    // Must be within bounds and empty
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return false;
    }
    if (getCell(row, col) != 0) return false;
    // Must flip at least one disc
    return getFlippedBitboard(row, col, isBlack) != 0;
}

std::vector<std::pair<int,int>> BitBoard::getValidMoves(bool isBlack) const {
    std::vector<std::pair<int,int>> moves;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (isValidMove(r, c, isBlack)) {
                moves.emplace_back(r, c);
            }
        }
    }
    return moves;
}

uint64_t BitBoard::getFlippedBitboard(int row, int col, bool isBlack) const {
    uint64_t flipped = 0;
    uint64_t playerBoard = getPlayerBoard(isBlack);
    uint64_t opponentBoard = getOpponentBoard(isBlack);
    // Directions: N, NE, E, SE, S, SW, W, NW
    static const std::array<std::pair<int,int>,8> directions = {
        std::make_pair(-1, 0),  // N
        std::make_pair(-1, 1),  // NE
        std::make_pair(0, 1),   // E
        std::make_pair(1, 1),   // SE
        std::make_pair(1, 0),   // S
        std::make_pair(1, -1),  // SW
        std::make_pair(0, -1),  // W
        std::make_pair(-1, -1)  // NW
    };
    for (auto [dr, dc] : directions) {
        int r = row + dr;
        int c = col + dc;
        uint64_t temp = 0;
        bool foundOpponent = false;
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
            uint64_t mask = positionToMask(r, c);
            if (opponentBoard & mask) {
                // Opponent piece found
                temp |= mask;
                foundOpponent = true;
            } else if (playerBoard & mask) {
                // Found player's own disc; only flip if we have seen at least one opponent disc
                if (foundOpponent) {
                    flipped |= temp;
                }
                break;
            } else {
                // Empty square: cannot flip along this line
                break;
            }
            r += dr;
            c += dc;
        }
    }
    return flipped;
}

std::vector<std::pair<int,int>> BitBoard::getFlippedDiscs(int row, int col, bool isBlack) const {
    std::vector<std::pair<int,int>> discs;
    uint64_t flipMask = getFlippedBitboard(row, col, isBlack);
    while (flipMask) {
        int bit = __builtin_ctzll(flipMask);
        auto [r, c] = bitToPosition(bit);
        discs.emplace_back(r, c);
        flipMask &= flipMask - 1;
    }
    return discs;
}

bool BitBoard::makeMove(int row, int col, bool isBlack) {
    if (!isValidMove(row, col, isBlack)) {
        return false;
    }
    uint64_t moveMask = positionToMask(row, col);
    uint64_t flipped = getFlippedBitboard(row, col, isBlack);
    if (isBlack) {
        blackBoard |= moveMask;
        // Flip opponent discs
        blackBoard |= flipped;
        whiteBoard &= ~flipped;
    } else {
        whiteBoard |= moveMask;
        whiteBoard |= flipped;
        blackBoard &= ~flipped;
    }
    return true;
}

bool BitBoard::hasValidMoves(bool isBlack) const {
    // Short circuit: if both boards fill the board, no moves remain
    if (isFull()) return false;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (isValidMove(r, c, isBlack)) {
                return true;
            }
        }
    }
    return false;
}

bool BitBoard::isGameOver() const {
    return !hasValidMoves(true) && !hasValidMoves(false);
}

bool BitBoard::isFull() const {
    // All 64 squares occupied if bit count equals 64
    return __builtin_popcountll(blackBoard | whiteBoard) == TOTAL_CELLS;
}

int BitBoard::getScore(bool isBlack) const {
    return __builtin_popcountll(isBlack ? blackBoard : whiteBoard);
}

int BitBoard::getTotalDiscs() const {
    return __builtin_popcountll(blackBoard | whiteBoard);
}

bool BitBoard::operator==(const BitBoard& other) const {
    return blackBoard == other.blackBoard && whiteBoard == other.whiteBoard;
}

bool BitBoard::operator!=(const BitBoard& other) const {
    return !(*this == other);
}

// Helper that shifts all bits in `board` by (dr,dc) while discarding bits
// that would cross the board edges.  This implementation explicitly
// computes row/col coordinates for each set bit.  Although slower than
// bitwise masks, it avoids wrap‑around and is easy to verify.
uint64_t BitBoard::shiftMask(uint64_t board, int dr, int dc) {
    uint64_t result = 0;
    while (board) {
        int bit = __builtin_ctzll(board);
        board &= board - 1;
        auto [r, c] = bitToPosition(bit);
        int nr = r + dr;
        int nc = c + dc;
        if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
            result |= positionToMask(nr, nc);
        }
    }
    return result;
}

// Initialise Zobrist table.  If a non‑zero seed is provided, use it to
// seed the random engine.  Otherwise use a random device.
void BitBoard::initializeZobrist(uint64_t seed) {
    if (zobristInitialised) return;
    std::mt19937_64 gen;
    if (seed == 0) {
        std::random_device rd;
        gen.seed(rd());
    } else {
        gen.seed(seed);
    }
    std::uniform_int_distribution<uint64_t> dist;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            for (int p = 0; p < 3; ++p) {
                zobristTable[r][c][p] = dist(gen);
            }
        }
    }
    zobristInitialised = true;
}

uint64_t BitBoard::getZobristKey(int row, int col, int player) {
    if (!zobristInitialised) {
        // Seed deterministically when called without explicit seed
        initializeZobrist();
    }
    return zobristTable[row][col][player];
}

uint64_t BitBoard::getZobristHash() const {
    if (!zobristInitialised) {
        initializeZobrist();
    }
    uint64_t hash = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            int state = getCell(r, c);
            // XOR only non‑empty cells; empty cells contribute zero
            if (state != 0) {
                hash ^= zobristTable[r][c][state];
            }
        }
    }
    return hash;
}