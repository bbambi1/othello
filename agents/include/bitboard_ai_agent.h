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

class BitBoardAIAgent : public AIAgentBase {
public:
  BitBoardAIAgent(const std::string &name = "BitBoard",
                  const std::string &author = "System", int maxDepth = 8);

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

  static const std::array<std::array<int, 8>, 8> POSITION_VALUES;
  static const std::array<std::array<int, 8>, 8> EARLY_GAME_VALUES;
  static const std::array<std::array<int, 8>, 8> LATE_GAME_VALUES;

  enum class EntryType { EXACT, LOWER_BOUND, UPPER_BOUND };
  struct TranspositionEntry {
    uint64_t hash;
    double score;
    int depth;
    EntryType type;
    std::chrono::steady_clock::time_point timestamp;
    TranspositionEntry()
        : hash(0), score(0.0), depth(0), type(EntryType::EXACT) {}
    TranspositionEntry(uint64_t h, double s, int d, EntryType t)
        : hash(h), score(s), depth(d), type(t) {}
  };
  static constexpr size_t TT_WAYS = 4;
  static constexpr size_t TT_NUM_BUCKETS = 1 << 20; // ~1M buckets
  struct TTBucket {
    TranspositionEntry entries[TT_WAYS];
  };
  mutable std::vector<TTBucket> transpositionTable;
  static_assert(TT_WAYS >= 2, "TT must be at least 2-way associative");

  uint64_t getZobristHash(const BitBoard &bitboard, bool blackToMove) const;
  void clearTranspositionTable();
  size_t getTranspositionTableSize() const;

  inline size_t ttBucketIndex(uint64_t hash) const {
    return static_cast<size_t>(hash % TT_NUM_BUCKETS);
  }
  const TranspositionEntry *ttLookup(uint64_t hash, int requiredDepth) const;
  void ttStore(uint64_t hash, double score, int depth, EntryType type) const;

  bool isBlackPlayer(CellState player) const;
  CellState playerFromBool(bool isBlack) const;

  std::vector<std::pair<int, int>>
  orderMoves(const BitBoard &bitboard,
             const std::vector<std::pair<int, int>> &moves, bool isBlack) const;

protected:
  bool isTimeUp(std::chrono::steady_clock::time_point startTime,
                std::chrono::milliseconds timeLimit =
                    std::chrono::milliseconds(1000)) const;
};
