#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <random>
#include <stdexcept>
#include <vector>

class BitBoard {
public:
  static constexpr int BOARD_SIZE = 8;
  static constexpr int TOTAL_CELLS = BOARD_SIZE * BOARD_SIZE;

  uint64_t blackBoard;
  uint64_t whiteBoard;

  BitBoard();
  BitBoard(uint64_t black, uint64_t white);

  BitBoard(const BitBoard &) = default;
  BitBoard &operator=(const BitBoard &) = default;
  bool operator<(const BitBoard &other) const;

  // Transformations
  BitBoard rot90() const;
  BitBoard rot180() const;
  BitBoard rot270() const;
  BitBoard mirrorHorizontal() const;
  BitBoard mirrorVertical() const;
  BitBoard mirrorDiag() const;
  BitBoard mirrorAntiDiag() const;
  BitBoard flipHorizontal() const;

  // Canonicalisation
  BitBoard toCanonical() const;

  bool isValidMove(int row, int col, bool isBlack) const;
  std::vector<std::pair<int, int>> getValidMoves(bool isBlack) const;
  bool makeMove(int row, int col, bool isBlack);
  bool isGameOver() const;
  bool hasValidMoves(bool isBlack) const;

  int getCell(int row, int col) const;
  void setCell(int row, int col, int state);

  int getScore(bool isBlack) const;
  int getTotalDiscs() const;

  bool isFull() const;
  void reset();

  static int positionToBit(int row, int col);
  static std::pair<int, int> bitToPosition(int bit);
  static uint64_t positionToMask(int row, int col);

  bool operator==(const BitBoard &other) const;
  bool operator!=(const BitBoard &other) const;

  uint64_t getPlayerBoard(bool isBlack) const;
  uint64_t getOpponentBoard(bool isBlack) const;

  uint64_t getCornerMask() const;
  uint64_t getEdgeMask() const;
  uint64_t getStableMask() const;

  static void initializeZobrist(uint64_t seed = 0);
  uint64_t getZobristHash() const;
  uint64_t getZobristHash(bool blackToMove) const;
  static uint64_t getZobristKey(int row, int col, int player);
  uint64_t getCanonicalZobristHash() const;

  uint64_t getStableMaskForPlayerBitwise(bool isBlack) const;
  bool isCornerControlled(int r, int c, bool isBlack) const;
  uint64_t getPlayerCorners(bool isBlack) const;

private:
  uint64_t getFlippedBitboard(int row, int col, bool isBlack) const;
  std::vector<std::pair<int, int>> getFlippedDiscs(int row, int col,
                                                   bool isBlack) const;

  static uint64_t shiftMask(uint64_t board, int dr, int dc);

  static std::array<std::array<std::array<uint64_t, 2>, 8>, 8> zobristTable;
  static uint64_t zobristBlackToMoveKey;
  static std::once_flag zobristOnce;
  static bool zobristInitialised;

  uint64_t hash_ = 0;

  static uint64_t flipHorizontal(uint64_t x) {
    const uint64_t k1 = 0x5555555555555555ULL;
    const uint64_t k2 = 0x3333333333333333ULL;
    const uint64_t k4 = 0x0f0f0f0f0f0f0f0fULL;
    x = ((x >> 1) & k1) | ((x & k1) << 1);
    x = ((x >> 2) & k2) | ((x & k2) << 2);
    x = ((x >> 4) & k4) | ((x & k4) << 4);
    return x;
  }

  static uint64_t flipVertical(uint64_t x) {
    const uint64_t k1 = 0x00FF00FF00FF00FFULL;
    const uint64_t k2 = 0x0000FFFF0000FFFFULL;
    x = ((x >> 8) & k1) | ((x & k1) << 8);
    x = ((x >> 16) & k2) | ((x & k2) << 16);
    x = (x >> 32) | (x << 32);
    return x;
  }

  static uint64_t flipDiagonal(uint64_t x) {
    uint64_t t;
    const uint64_t k1 = 0x5500550055005500ULL;
    const uint64_t k2 = 0x3333000033330000ULL;
    const uint64_t k4 = 0x0f0f0f0f00000000ULL;

    t = k4 & (x ^ (x << 28));
    x ^= t ^ (t >> 28);
    t = k2 & (x ^ (x << 14));
    x ^= t ^ (t >> 14);
    t = k1 & (x ^ (x << 7));
    x ^= t ^ (t >> 7);
    return x;
  }

  static uint64_t flipAntiDiagonal(uint64_t x) {
    uint64_t result = 0ULL;
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        int fromBit = r * 8 + c;
        int toBit = (7 - c) * 8 + (7 - r); // Flip anti-diagonal
        if (x & (1ULL << fromBit)) {
          result |= (1ULL << toBit);
        }
      }
    }
    return result;
  }
};
