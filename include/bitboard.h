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
  static uint64_t getZobristKey(int row, int col, int player01);

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
};
