#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <mutex>
#include <stdexcept>
#include <vector>

class BitBoard {
public:
  static constexpr int BOARD_SIZE = 8;
  static constexpr int TOTAL_CELLS = BOARD_SIZE * BOARD_SIZE;

  // Bitboards: bits set where discs are present for each colour
  uint64_t blackBoard;
  uint64_t whiteBoard;

  BitBoard();
  BitBoard(uint64_t black, uint64_t white);

  BitBoard(const BitBoard &) = default;
  BitBoard &operator=(const BitBoard &) = default;

  // Query whether a given move is legal for `isBlack`.  A legal move
  // must lie inside the board, land on an empty square and flip at
  // least one opposing disc.
  bool isValidMove(int row, int col, bool isBlack) const;
  // Collect all valid moves for the specified colour.
  std::vector<std::pair<int, int>> getValidMoves(bool isBlack) const;
  // Apply a move, flipping all appropriate discs.  Returns true if
  // the move was legal and applied; false if the move was illegal.
  bool makeMove(int row, int col, bool isBlack);
  // Returns true when neither player has a legal move.
  bool isGameOver() const;
  // Returns true if the given colour has at least one legal move.
  bool hasValidMoves(bool isBlack) const;

  // Cell access: 0 = empty, 1 = black, 2 = white
  int getCell(int row, int col) const;
  void setCell(int row, int col, int state);

  // Score counts for each player
  int getScore(bool isBlack) const;
  int getTotalDiscs() const;

  // Board tests
  bool isFull() const;
  void reset();

  // Helper: convert (row,col) to bit index and mask
  static int positionToBit(int row, int col);
  static std::pair<int, int> bitToPosition(int bit);
  static uint64_t positionToMask(int row, int col);

  // Comparisons
  bool operator==(const BitBoard &other) const;
  bool operator!=(const BitBoard &other) const;

  // Boards for the current player and the opponent
  uint64_t getPlayerBoard(bool isBlack) const;
  uint64_t getOpponentBoard(bool isBlack) const;

  // Simple bit masks for evaluation functions.
  // A bit mask of the four corner squares (0,0), (0,7), (7,0), (7,7).
  uint64_t getCornerMask() const;
  // A bit mask covering all edge squares (top, bottom, left and right
  // edges) excluding the four corners.
  uint64_t getEdgeMask() const;
  // Placeholder for stability mask: currently returns zero.  A proper
  // implementation would mark discs that cannot be flipped.
  uint64_t getStableMask() const;

  // Zobrist hashing: deterministic if `seed` supplied on first call
  static void initializeZobrist(uint64_t seed = 0);
  // Returns the piece-only hash (no side-to-move). Maintained incrementally.
  uint64_t getZobristHash() const;
  // Returns full hash including side-to-move when `blackToMove` is true.
  uint64_t getZobristHash(bool blackToMove) const;
  static uint64_t getZobristKey(int row, int col, int player01);

private:
  // Computes all discs flipped by placing a disc at (row,col).  A
  // flipped disc is one that lies on a contiguous line of opponent
  // pieces ended by a friendly piece.
  uint64_t getFlippedBitboard(int row, int col, bool isBlack) const;
  std::vector<std::pair<int, int>> getFlippedDiscs(int row, int col,
                                                   bool isBlack) const;

  // Bitwise shift that safely moves all bits in `board` by the
  // specified (dr, dc) offset.  Bits that would leave the board are
  // discarded.  `dr` is the change in row (-1 = up, +1 = down), `dc`
  // is the change in column (-1 = left, +1 = right).  This helper
  // avoids wrap‑around and undefined behaviour from shifting by large
  // amounts.
  static uint64_t shiftMask(uint64_t board, int dr, int dc);

  // Zobrist table: row × col × (0=black,1=white)
  static std::array<std::array<std::array<uint64_t, 2>, 8>, 8> zobristTable;
  static uint64_t zobristBlackToMoveKey;
  static std::once_flag zobristOnce;
  static bool zobristInitialised;

  // Incremental piece-only hash
  uint64_t hash_ = 0;
};
