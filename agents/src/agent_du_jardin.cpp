#include "agent_du_jardin.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>

bool phase2 = false;

AgentDuJardin::AgentDuJardin(const std::string &name, const std::string &author,
                             int depth)
    : AIAgentBase(name, author), maxDepth(depth) {}

inline std::pair<int, int>
AgentDuJardin::getBestMove(const Board &board, CellState player,
                           std::chrono::milliseconds timeLimit) {
  auto startTime = std::chrono::steady_clock::now();
  transpositionTable.clear();
  BitBoard bitboard;
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      CellState cell = board.getCell(row, col);
      if (cell == CellState::BLACK) {
        bitboard.setCell(row, col, 1);
      } else if (cell == CellState::WHITE) {
        bitboard.setCell(row, col, 2);
      }
    }
  }
  bool isBlackTurn = (player == CellState::BLACK);
  auto moves = bitboard.getValidMoves(isBlackTurn);
  if (moves.empty())
    return {-1, -1};
  moves = orderMoves(bitboard, moves, isBlackTurn);
  std::pair<int, int> bestMove = moves.front();
  double bestScore = std::numeric_limits<double>::lowest();

  int depth = computeDepth(bitboard, isBlackTurn);

  // int depth = maxDepth - 1;

  // phase2 = bitboard.getTotalDiscs() > 20;
  phase2 = bitboard.getTotalDiscs() > 4;

  for (const auto &mv : moves) {
    if (isTimeUpLarge(startTime, timeLimit))
      break;
    BitBoard temp = bitboard;
    if (temp.makeMove(mv.first, mv.second, isBlackTurn)) {
      double score =
          bitboardMinMax(temp, depth, std::numeric_limits<double>::lowest(),
                         std::numeric_limits<double>::max(), isBlackTurn, false,
                         startTime, timeLimit);
      if (score > bestScore) {
        bestScore = score;
        bestMove = mv;
      }
    }
  }

  return bestMove;
}

int AgentDuJardin::computeDepth(const BitBoard &bitboard, bool isBlack) const {
  (void)isBlack; // Suppress unused parameter warning
  if (bitboard.getTotalDiscs() < 20) {
    return maxDepth;
  }
  return maxDepth - 1;
}

inline double
AgentDuJardin::bitboardMinMax(BitBoard &bitboard, int depth, double alpha,
                              double beta, bool isBlack, bool isMaximizing,
                              std::chrono::steady_clock::time_point startTime,
                              std::chrono::milliseconds timeLimit) {
  if (isTimeUpLarge(startTime, timeLimit))
    return 0.0;
  uint64_t hash = getZobristHash(bitboard);
  auto it = transpositionTable.find(hash);
  if (it != transpositionTable.end()) {
    const TranspositionEntry &entry = it->second[isMaximizing ? 1 : 0];
    if (entry.depth >= depth && entry.isMaximizing == isMaximizing &&
        entry.depth != -1) {
      switch (entry.type) {
      case EntryType::EXACT:
        return entry.score;
      case EntryType::LOWER_BOUND:
        if (entry.score >= beta) {
          return entry.score;
        }
        break;
      case EntryType::UPPER_BOUND:
        if (entry.score <= alpha) {
          return entry.score;
        }
        break;
      }
    }
  }
  if (depth == 0) {
    double sc = evaluateBitboard(bitboard, isBlack);
    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE && phase2) {
      transpositionTable[hash][isMaximizing ? 1 : 0] =
          TranspositionEntry(hash, sc, depth, EntryType::EXACT, isMaximizing);
    }
    return sc;
  }
  if (bitboard.isGameOver()) {
    double sc = bitboard.getScore(isBlack) > bitboard.getScore(!isBlack)
                    ? std::numeric_limits<double>::max()
                    : std::numeric_limits<double>::lowest();
    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE && phase2) {
      transpositionTable[hash][isMaximizing ? 1 : 0] =
          TranspositionEntry(hash, sc, depth, EntryType::EXACT, isMaximizing);
    }
    return sc;
  }
  auto moves = bitboard.getValidMoves(isMaximizing ? isBlack : !isBlack);
  if (moves.empty()) {
    double sc = bitboardMinMax(bitboard, depth - 1, alpha, beta, isBlack,
                               !isMaximizing, startTime, timeLimit);
    return sc;
  }
  moves = orderMoves(bitboard, moves, isMaximizing ? isBlack : !isBlack);
  if (isMaximizing) {
    double best = std::numeric_limits<double>::lowest();
    EntryType eType = EntryType::UPPER_BOUND;
    for (const auto &mv : moves) {
      if (isTimeUpLarge(startTime, timeLimit))
        return best;
      BitBoard temp = bitboard;
      if (temp.makeMove(mv.first, mv.second, isBlack)) {
        double sc = bitboardMinMax(temp, depth - 1, alpha, beta, isBlack, false,
                                   startTime, timeLimit);
        best = std::max(best, sc);
        alpha = std::max(alpha, sc);
        if (beta <= alpha) {
          if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE && phase2) {
            transpositionTable[hash][isMaximizing ? 1 : 0] = TranspositionEntry(
                hash, best, depth, EntryType::UPPER_BOUND, isMaximizing);
          }
          break;
        }
      }
    }
    if (best <= alpha)
      eType = EntryType::UPPER_BOUND;
    else if (best >= beta)
      eType = EntryType::LOWER_BOUND;
    else
      eType = EntryType::EXACT;
    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE && phase2) {
      transpositionTable[hash][isMaximizing ? 1 : 0] =
          TranspositionEntry(hash, best, depth, eType, isMaximizing);
    }
    return best;
  } else {
    double best = std::numeric_limits<double>::max();
    EntryType eType = EntryType::LOWER_BOUND;
    for (const auto &mv : moves) {
      if (isTimeUpLarge(startTime, timeLimit))
        return best;
      BitBoard temp = bitboard;
      if (temp.makeMove(mv.first, mv.second, !isBlack)) {
        double sc = bitboardMinMax(temp, depth - 1, alpha, beta, isBlack, true,
                                   startTime, timeLimit);
        best = std::min(best, sc);
        beta = std::min(beta, sc);
        if (beta <= alpha) {
          if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE && phase2) {
            transpositionTable[hash][isMaximizing ? 1 : 0] = TranspositionEntry(
                hash, best, depth, EntryType::LOWER_BOUND, isMaximizing);
          }
          break;
        }
      }
    }
    if (best <= alpha)
      eType = EntryType::UPPER_BOUND;
    else if (best >= beta)
      eType = EntryType::LOWER_BOUND;
    else
      eType = EntryType::EXACT;
    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE && phase2) {
      transpositionTable[hash][isMaximizing ? 1 : 0] =
          TranspositionEntry(hash, best, depth, eType, isMaximizing);
    }
    return best;
  }
}

inline double AgentDuJardin::evaluateBitboard(const BitBoard &bitboard,
                                              bool isBlack) const {
  double sc = 0.0;
  sc += evaluateCornerControlBitboard(bitboard, isBlack) * 25.0;
  sc += evaluateEdgeControlBitboard(bitboard, isBlack) * 5.0;
  sc += evaluateMobilityBitboard(bitboard, isBlack) * 15.0;
  sc += evaluateDiscCountBitboard(bitboard, isBlack) * 5.0;
  sc += evaluateStabilityBitboard(bitboard, isBlack) * 10.0;
  return sc;
}

inline double AgentDuJardin::evaluateStabilityBitboard(const BitBoard &bb,
                                                       bool isBlack) const {
  uint64_t pstable = bb.getStableMaskForPlayerBitwise(isBlack);
  int nb_pstable = __builtin_popcountll(pstable);
  uint64_t ostable = bb.getStableMaskForPlayerBitwise(!isBlack);
  int nb_ostable = __builtin_popcountll(ostable);
  if (nb_ostable + nb_pstable == 0) {
    return 0.0;
  }
  double sc =
      static_cast<double>(nb_pstable - nb_ostable) / (nb_pstable + nb_ostable);
  return sc;
}

inline double AgentDuJardin::evaluateCornerControlBitboard(const BitBoard &bb,
                                                           bool isBlack) const {
  uint64_t corners = bb.getCornerMask();
  uint64_t player = bb.getPlayerBoard(isBlack);
  uint64_t opponent = bb.getOpponentBoard(isBlack);
  int pc = __builtin_popcountll(player & corners);
  int oc = __builtin_popcountll(opponent & corners);
  return static_cast<double>(pc - oc);
}

inline double AgentDuJardin::evaluateEdgeControlBitboard(const BitBoard &bb,
                                                         bool isBlack) const {
  uint64_t edges = bb.getEdgeMask();
  uint64_t player = bb.getPlayerBoard(isBlack);
  uint64_t opponent = bb.getOpponentBoard(isBlack);
  int pe = __builtin_popcountll(player & edges);
  int oe = __builtin_popcountll(opponent & edges);
  return static_cast<double>(pe - oe);
}

inline double AgentDuJardin::evaluateMobilityBitboard(const BitBoard &bb,
                                                      bool isBlack) const {
  int pMoves = bb.getValidMoves(isBlack).size();
  int oMoves = bb.getValidMoves(!isBlack).size();
  if (pMoves + oMoves == 0)
    return 0.0;
  return static_cast<double>(pMoves - oMoves) / (pMoves + oMoves);
}

inline double AgentDuJardin::evaluateDiscCountBitboard(const BitBoard &bb,
                                                       bool isBlack) const {
  int p = bb.getScore(isBlack);
  int o = bb.getScore(!isBlack);
  int t = bb.getTotalDiscs();
  if (t == 0)
    return 0.0;
  return static_cast<double>(p - o) / t;
}

inline uint64_t AgentDuJardin::getZobristHash(const BitBoard &bb) const {
  return bb.getCanonicalZobristHash();
}

inline void AgentDuJardin::clearTranspositionTable() {
  transpositionTable.clear();
}

inline size_t AgentDuJardin::getTranspositionTableSize() const {
  return transpositionTable.size();
}

inline bool AgentDuJardin::isBlackPlayer(CellState p) const {
  return p == CellState::BLACK;
}

inline CellState AgentDuJardin::playerFromBool(bool isBlack) const {
  return isBlack ? CellState::BLACK : CellState::WHITE;
}

inline std::vector<std::pair<int, int>>
AgentDuJardin::orderMoves(const BitBoard &bb,
                          const std::vector<std::pair<int, int>> &moves,
                          bool isBlack) const {
  std::vector<std::pair<std::pair<int, int>, double>> moveScores;

  uint64_t playerCorners = bb.getPlayerCorners(isBlack);

  auto isCornerControlled = [&](int r, int c) {
    return (playerCorners & (1ULL << (r * 8 + c))) != 0;
  };

  for (const auto &mv : moves) {
    double score = 0.0;

    // Corner
    if ((mv.first == 0 || mv.first == 7) &&
        (mv.second == 0 || mv.second == 7)) {
      score += 1000.0;
    } else if (isXSquare(mv.first, mv.second)) {
      int corner_r = (mv.first < 4) ? 0 : 7;
      int corner_c = (mv.second < 4) ? 0 : 7;
      if (!isCornerControlled(corner_r, corner_c)) {
        score -= 500.0;
      }
    } else if (isCSquare(mv.first, mv.second)) {
      int corner_r = (mv.first < 4) ? 0 : 7;
      int corner_c = (mv.second < 4) ? 0 : 7;
      if (!isCornerControlled(corner_r, corner_c)) {
        score -= 250.0;
      }
    }
    // Bords autres que coins
    else if (mv.first == 0 || mv.first == 7 || mv.second == 0 ||
             mv.second == 7) {
      score += 100.0;
    }
    BitBoard tmp = bb;
    int before = bb.getScore(isBlack);
    if (tmp.makeMove(mv.first, mv.second, isBlack)) {
      int after = tmp.getScore(isBlack);
      int flips = after - before - 1;
      if (flips > 0)
        score += flips * 10.0;

      if (tmp.getValidMoves(!isBlack).empty())
        score += 1000;
    }

    moveScores.emplace_back(mv, score);
  }

  std::sort(moveScores.begin(), moveScores.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  std::vector<std::pair<int, int>> ordered;
  ordered.reserve(moveScores.size());
  for (const auto &ms : moveScores)
    ordered.push_back(ms.first);

  return ordered;
}

inline bool
AgentDuJardin::isTimeUpLarge(std::chrono::steady_clock::time_point startTime,
                             std::chrono::milliseconds timeLimit) const {
  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);

  auto threshold = std::chrono::milliseconds(
      static_cast<long long>(timeLimit.count() * 0.95));
  return elapsed >= threshold;
}

REGISTER_AI_AGENT(AgentDuJardin, "agentDuJardin")
