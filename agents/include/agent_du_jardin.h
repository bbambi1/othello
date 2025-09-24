#pragma once

#include "ai_agent_base.h"
#include "bitboard.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <random>
#include <unordered_map>
#include <vector>

class Board;
enum class CellState;

class AgentDuJardin : public AIAgentBase {
public:
  AgentDuJardin(const std::string &name = "AgentDuJardin",
                const std::string &author = "Alexis", int maxDepth = 8);

  std::pair<int, int> getBestMove(const Board &board, CellState player,
                                  std::chrono::milliseconds timeLimit =
                                      std::chrono::milliseconds(1000)) override;

private:
  int maxDepth;

  double bitboardMinMax(BitBoard &bitboard, int depth, double alpha,
                        double beta, bool isBlack, bool isMaximizing,
                        std::chrono::steady_clock::time_point startTime,
                        std::chrono::milliseconds timeLimit);

  double evaluateBitboard(const BitBoard &bitboard, bool isBlack) const;
  double evaluateCornerControlBitboard(const BitBoard &bitboard,
                                       bool isBlack) const;
  double evaluateEdgeControlBitboard(const BitBoard &bitboard,
                                     bool isBlack) const;
  double evaluateMobilityBitboard(const BitBoard &bitboard, bool isBlack) const;
  double evaluateStabilityBitboard(const BitBoard &bitboard,
                                   bool isBlack) const;
  double evaluateDiscCountBitboard(const BitBoard &bitboard,
                                   bool isBlack) const;

  int computeDepth(const BitBoard &bitboard, bool isBlack) const;

  bool updateDepth(std::chrono::steady_clock::time_point startTime,
                   std::chrono::milliseconds timeLimit =
                       std::chrono::milliseconds(1000)) const;

  static const std::array<std::array<int, 8>, 8> POSITION_VALUES;
  static const std::array<std::array<int, 8>, 8> EARLY_GAME_VALUES;
  static const std::array<std::array<int, 8>, 8> LATE_GAME_VALUES;

  // Positions X squares (cases diagonales adjacentes aux coins)
  static constexpr uint64_t X_SQUARES = (1ULL << (1 * 8 + 1)) | // (1,1)
                                        (1ULL << (1 * 8 + 6)) | // (1,6)
                                        (1ULL << (6 * 8 + 1)) | // (6,1)
                                        (1ULL << (6 * 8 + 6));  // (6,6)

  // Positions C squares (cases adjacentes aux coins sur les bords)
  static constexpr uint64_t C_SQUARES = (1ULL << (0 * 8 + 1)) | // (0,1)
                                        (1ULL << (1 * 8 + 0)) | // (1,0)
                                        (1ULL << (0 * 8 + 6)) | // (0,6)
                                        (1ULL << (1 * 8 + 7)) | // (1,7)
                                        (1ULL << (6 * 8 + 0)) | // (6,0)
                                        (1ULL << (7 * 8 + 1)) | // (7,1)
                                        (1ULL << (6 * 8 + 7)) | // (6,7)
                                        (1ULL << (7 * 8 + 6));  // (7,6)

  inline bool isXSquare(int r, int c) const {
    return (X_SQUARES & (1ULL << (r * 8 + c))) != 0;
  }

  inline bool isCSquare(int r, int c) const {
    return (C_SQUARES & (1ULL << (r * 8 + c))) != 0;
  }

  enum class EntryType { EXACT, LOWER_BOUND, UPPER_BOUND };
  struct TranspositionEntry {
    uint64_t hash;
    double score;
    int depth;
    EntryType type;
    bool isMaximizing;
    std::chrono::steady_clock::time_point timestamp;
    TranspositionEntry()
        : hash(0), score(0.0), depth(-1), type(EntryType::EXACT),
          isMaximizing(true) {}
    TranspositionEntry(uint64_t h, double s, int d, EntryType t, bool b)
        : hash(h), score(s), depth(d), type(t), isMaximizing(b) {}
  };
  struct Etat {
    BitBoard bitboard;
    bool isMaximizing;
    Etat(BitBoard bb, bool b) : bitboard(bb), isMaximizing(b) {}
  };
  // mutable std::unordered_map<uint64_t,TranspositionEntry> transpositionTable;
  mutable std::unordered_map<uint64_t, std::array<TranspositionEntry, 2>>
      transpositionTable;
  static constexpr size_t MAX_TRANSPOSITION_SIZE = 4000000;

  uint64_t getZobristHash(const BitBoard &bitboard) const;
  void clearTranspositionTable();
  size_t getTranspositionTableSize() const;

  bool isBlackPlayer(CellState player) const;
  CellState playerFromBool(bool isBlack) const;

  std::vector<std::pair<int, int>>
  orderMoves(const BitBoard &bitboard,
             const std::vector<std::pair<int, int>> &moves, bool isBlack) const;

  bool isTimeUpLarge(std::chrono::steady_clock::time_point startTime,
                     std::chrono::milliseconds timeLimit) const;

  // Quiescence search placeholder (not implemented yet)
  // double quiescenceSearch(BitBoard& bitboard, double alpha, double beta,
  //                         bool isBlack, std::chrono::steady_clock::time_point
  //                         startTime, std::chrono::milliseconds timeLimit)
  //                         const { return 0.0; }
};
