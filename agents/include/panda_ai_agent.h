#pragma once

#include "ai_agent_base.h"
#include "bitboard.h"
#include <array>
#include <chrono>
#include <limits>
#include <unordered_map>
#include <vector>

class Board;
enum class CellState;

class PandaAIAgent : public AIAgentBase {
public:
  PandaAIAgent(const std::string &name = "Panda",
               const std::string &author = "PandaAI-Dev", int maxDepth = 12);

  std::pair<int, int> getBestMove(const Board &board, CellState player,
                                  std::chrono::milliseconds timeLimit =
                                      std::chrono::milliseconds(1000)) override;

private:
  int maxDepth;
  static constexpr int ENDGAME_DEPTH = 16;
  static constexpr size_t TT_SIZE = 2097152;

  mutable uint64_t nodesSearched;

  enum class GamePhase { OPENING, MIDGAME, ENDGAME };
  GamePhase getGamePhase(const BitBoard &bitboard) const;

  std::pair<int, int>
  iterativeDeepeningSearch(const BitBoard &bitboard, bool isBlack,
                           std::chrono::steady_clock::time_point startTime,
                           std::chrono::milliseconds timeLimit);

  double negamax(BitBoard &bitboard, int depth, double alpha, double beta,
                 bool isBlack, std::chrono::steady_clock::time_point startTime,
                 std::chrono::milliseconds timeLimit, bool &timeUp);

  double exactEndgameSearch(BitBoard &bitboard, bool isBlack, int emptySquares,
                            std::chrono::steady_clock::time_point startTime,
                            std::chrono::milliseconds timeLimit, bool &timeUp);

  // Evaluation system
  double evaluatePosition(const BitBoard &bitboard, bool isBlack) const;
  double evaluateMobility(const BitBoard &bitboard, bool isBlack) const;
  double evaluateCornerControl(const BitBoard &bitboard, bool isBlack) const;
  double evaluateEdgeControl(const BitBoard &bitboard, bool isBlack) const;
  double evaluateStability(const BitBoard &bitboard, bool isBlack) const;
  double evaluateDiscCount(const BitBoard &bitboard, bool isBlack) const;
  double evaluatePositional(const BitBoard &bitboard, bool isBlack) const;

  struct EvaluationWeights {
    double mobility;
    double cornerControl;
    double edgeControl;
    double stability;
    double discCount;
    double positional;
  };

  EvaluationWeights getWeights(GamePhase phase) const;

  // Transposition table
  enum class EntryType { EXACT, LOWER_BOUND, UPPER_BOUND };
  struct TTEntry {
    uint64_t hash;
    double score;
    int depth;
    EntryType type;
    std::pair<int, int> bestMove;

    TTEntry()
        : hash(0), score(0.0), depth(0), type(EntryType::EXACT),
          bestMove({-1, -1}) {}
    TTEntry(uint64_t h, double s, int d, EntryType t, std::pair<int, int> move)
        : hash(h), score(s), depth(d), type(t), bestMove(move) {}
  };

  mutable std::vector<TTEntry> transpositionTable;

  mutable std::array<int, 64> historyTable = {0};

  bool probeTTEntry(uint64_t hash, int depth, double alpha, double beta,
                    double &score, std::pair<int, int> &bestMove) const;
  void storeTTEntry(uint64_t hash, double score, int depth, EntryType type,
                    std::pair<int, int> bestMove) const;

  // Move ordering
  std::vector<std::pair<int, int>>
  orderMoves(const BitBoard &bitboard,
             const std::vector<std::pair<int, int>> &moves, bool isBlack,
             std::pair<int, int> ttMove = {-1, -1}) const;

  double scoreMoveForOrdering(const BitBoard &bitboard,
                              std::pair<int, int> move, bool isBlack) const;

  // Opening book
  std::pair<int, int> getOpeningMove(const BitBoard &bitboard,
                                     bool isBlack) const;
  bool isOpeningPosition(const BitBoard &bitboard) const;

  std::pair<int, int>
  selectStrategicOpeningMove(const BitBoard &bitboard,
                             const std::vector<std::pair<int, int>> &validMoves,
                             bool isBlack) const;

  // Utility methods
  bool isCorner(int row, int col) const;
  bool isEdge(int row, int col) const;
  bool isTimeUp(std::chrono::steady_clock::time_point startTime,
                std::chrono::milliseconds timeLimit) const;

  // Bitboard conversion
  BitBoard boardToBitboard(const Board &board) const;

  // Position evaluation tables
  static const std::array<std::array<int, 8>, 8> POSITION_VALUES;
  static const std::array<std::array<int, 8>, 8> OPENING_VALUES;
  static const std::array<std::array<int, 8>, 8> ENDGAME_VALUES;
};
