#include "panda_ai_agent.h"
#include <algorithm>
#include <cmath>
#include <random>

const std::array<std::array<int, 8>, 8> PandaAIAgent::POSITION_VALUES = {
    {{{120, -20, 20, 5, 5, 20, -20, 120}},
     {{-20, -40, -5, -5, -5, -5, -40, -20}},
     {{20, -5, 15, 3, 3, 15, -5, 20}},
     {{5, -5, 3, 3, 3, 3, -5, 5}},
     {{5, -5, 3, 3, 3, 3, -5, 5}},
     {{20, -5, 15, 3, 3, 15, -5, 20}},
     {{-20, -40, -5, -5, -5, -5, -40, -20}},
     {{120, -20, 20, 5, 5, 20, -20, 120}}}};

const std::array<std::array<int, 8>, 8> PandaAIAgent::OPENING_VALUES = {
    {{{100, -20, 10, 5, 5, 10, -20, 100}},
     {{-20, -50, -2, -2, -2, -2, -50, -20}},
     {{10, -2, -1, -1, -1, -1, -2, 10}},
     {{5, -2, -1, -1, -1, -1, -2, 5}},
     {{5, -2, -1, -1, -1, -1, -2, 5}},
     {{10, -2, -1, -1, -1, -1, -2, 10}},
     {{-20, -50, -2, -2, -2, -2, -50, -20}},
     {{100, -20, 10, 5, 5, 10, -20, 100}}}};

const std::array<std::array<int, 8>, 8> PandaAIAgent::ENDGAME_VALUES = {
    {{{100, 50, 30, 20, 20, 30, 50, 100}},
     {{50, 30, 20, 10, 10, 20, 30, 50}},
     {{30, 20, 10, 5, 5, 10, 20, 30}},
     {{20, 10, 5, 0, 0, 5, 10, 20}},
     {{20, 10, 5, 0, 0, 5, 10, 20}},
     {{30, 20, 10, 5, 5, 10, 20, 30}},
     {{50, 30, 20, 10, 10, 20, 30, 50}},
     {{100, 50, 30, 20, 20, 30, 50, 100}}}};

PandaAIAgent::PandaAIAgent(const std::string &name, const std::string &author,
                           int depth)
    : AIAgentBase(name, author), maxDepth(depth), nodesSearched(0) {
  transpositionTable.resize(TT_SIZE);
}

std::pair<int, int>
PandaAIAgent::getBestMove(const Board &board, CellState player,
                          std::chrono::milliseconds timeLimit) {
  auto startTime = std::chrono::steady_clock::now();
  nodesSearched = 0;

  for (int i = 0; i < 64; ++i)
    historyTable[i] = 0;

  BitBoard bitboard = boardToBitboard(board);
  bool isBlack = (player == CellState::BLACK);

  auto validMoves = bitboard.getValidMoves(isBlack);
  if (validMoves.empty()) {
    return {-1, -1};
  }

  if (validMoves.size() == 1) {
    return validMoves[0];
  }

  if (isOpeningPosition(bitboard)) {
    auto openingMove = getOpeningMove(bitboard, isBlack);
    if (openingMove.first != -1) {
      return openingMove;
    }
  }

  return iterativeDeepeningSearch(bitboard, isBlack, startTime, timeLimit);
}

BitBoard PandaAIAgent::boardToBitboard(const Board &board) const {
  BitBoard bitboard;
  bitboard.blackBoard = 0;
  bitboard.whiteBoard = 0;

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

  return bitboard;
}

PandaAIAgent::GamePhase
PandaAIAgent::getGamePhase(const BitBoard &bitboard) const {
  int totalDiscs = bitboard.getTotalDiscs();

  if (totalDiscs <= 20)
    return GamePhase::OPENING;
  if (totalDiscs >= 50)
    return GamePhase::ENDGAME;
  return GamePhase::MIDGAME;
}

std::pair<int, int> PandaAIAgent::iterativeDeepeningSearch(
    const BitBoard &bitboard, bool isBlack,
    std::chrono::steady_clock::time_point startTime,
    std::chrono::milliseconds timeLimit) {

  auto validMoves = bitboard.getValidMoves(isBlack);
  std::pair<int, int> bestMove = validMoves[0];
  double bestScore = -std::numeric_limits<double>::infinity();

  // Allocate 95% of time for iterative deepening
  auto searchTimeLimit =
      std::chrono::milliseconds(static_cast<long>(timeLimit.count() * 0.95));

  // Check for endgame exact search
  int emptySquares = 64 - bitboard.getTotalDiscs();
  if (emptySquares <= ENDGAME_DEPTH) {
    bool timeUp = false;

    for (const auto &move : validMoves) {
      if (isTimeUp(startTime, timeLimit))
        break;

      BitBoard tempBoard = bitboard;
      if (tempBoard.makeMove(move.first, move.second, isBlack)) {
        double score =
            -exactEndgameSearch(tempBoard, !isBlack, emptySquares - 1,
                                startTime, timeLimit, timeUp);
        if (score > bestScore) {
          bestScore = score;
          bestMove = move;
        }
      }
    }
    return bestMove;
  }

  // Iterative deepening search - start from depth 1 for better exploration
  for (int depth = 1; depth <= maxDepth; ++depth) {
    if (isTimeUp(startTime, searchTimeLimit))
      break;

    double iterationBestScore = -std::numeric_limits<double>::infinity();
    std::pair<int, int> iterationBestMove = validMoves[0];
    bool timeUp = false;

    // Order moves based on previous iteration results
    auto orderedMoves = orderMoves(bitboard, validMoves, isBlack, bestMove);

    // Aspiration window around previous best score (±50)
    double baseAlpha = -std::numeric_limits<double>::infinity();
    double baseBeta = std::numeric_limits<double>::infinity();
    const double aspWindow = 50.0; // "centipawn"-ish units
    if (depth > 1 && !std::isinf(bestScore)) {
      baseAlpha = bestScore - aspWindow;
      baseBeta = bestScore + aspWindow;
    }

    int attempts = 0;
    const int maxAttempts = 3;
    bool complete = false;
    while (!complete && attempts < maxAttempts) {
      attempts++;
      double alpha = baseAlpha;
      double beta = baseBeta;

      iterationBestScore = -std::numeric_limits<double>::infinity();
      iterationBestMove = orderedMoves[0];
      timeUp = false;

      for (const auto &move : orderedMoves) {
        if (isTimeUp(startTime, searchTimeLimit)) {
          timeUp = true;
          break;
        }

        BitBoard tempBoard = bitboard;
        if (tempBoard.makeMove(move.first, move.second, isBlack)) {
          bool searchTimeUp = false;
          double score = -negamax(tempBoard, depth - 1, -beta, -alpha, !isBlack,
                                  startTime, searchTimeLimit, searchTimeUp);

          if (score > iterationBestScore) {
            iterationBestScore = score;
            iterationBestMove = move;
          }

          alpha = std::max(alpha, score);
          if (alpha >= beta) {
            // cutoff
            break;
          }
        }
      }

      if (timeUp)
        break; // give up on this depth

      // Check aspiration window result and decide whether to widen
      if (iterationBestScore <= baseAlpha) {
        // Fail low: widen window downwards
        baseAlpha -= aspWindow * 2.0;
        continue; // retry
      } else if (iterationBestScore >= baseBeta) {
        // Fail high: widen window upwards
        baseBeta += aspWindow * 2.0;
        continue; // retry
      } else {
        complete = true; // within window
      }
    }

    // Only update best move if we completed the iteration (no time out)
    if (!timeUp) {
      bestScore = iterationBestScore;
      bestMove = iterationBestMove;
    }
  }

  return bestMove;
}

bool PandaAIAgent::isTimeUp(std::chrono::steady_clock::time_point startTime,
                            std::chrono::milliseconds timeLimit) const {
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - startTime);
  return elapsed >= timeLimit;
}

bool PandaAIAgent::isCorner(int row, int col) const {
  return (row == 0 || row == 7) && (col == 0 || col == 7);
}

bool PandaAIAgent::isEdge(int row, int col) const {
  return row == 0 || row == 7 || col == 0 || col == 7;
}

double PandaAIAgent::negamax(BitBoard &bitboard, int depth, double alpha,
                             double beta, bool isBlack,
                             std::chrono::steady_clock::time_point startTime,
                             std::chrono::milliseconds timeLimit,
                             bool &timeUp) {
  nodesSearched++;

  // Time check every 2000 nodes for better performance
  if (nodesSearched % 2000 == 0 && isTimeUp(startTime, timeLimit)) {
    timeUp = true;
    return 0.0;
  }

  // Probe transposition table
  uint64_t hash = bitboard.getZobristHash();
  std::pair<int, int> ttMove;
  double ttScore;
  if (probeTTEntry(hash, depth, alpha, beta, ttScore, ttMove)) {
    return ttScore;
  }

  // Terminal node evaluation
  if (depth == 0 || bitboard.isGameOver()) {
    double score = evaluatePosition(bitboard, isBlack);
    storeTTEntry(hash, score, depth, EntryType::EXACT, {-1, -1});
    return score;
  }

  auto validMoves = bitboard.getValidMoves(isBlack);
  if (validMoves.empty()) {
    // Pass turn
    if (!bitboard.hasValidMoves(!isBlack)) {
      // Game over
      double score = evaluatePosition(bitboard, isBlack);
      storeTTEntry(hash, score, depth, EntryType::EXACT, {-1, -1});
      return score;
    }

    // Recursive call with passed turn
    double score = -negamax(bitboard, depth - 1, -beta, -alpha, !isBlack,
                            startTime, timeLimit, timeUp);
    storeTTEntry(hash, score, depth, EntryType::EXACT, {-1, -1});
    return score;
  }

  // Order moves for better alpha-beta pruning
  validMoves = orderMoves(bitboard, validMoves, isBlack, ttMove);

  double bestScore = -std::numeric_limits<double>::infinity();
  std::pair<int, int> bestMove = validMoves[0];
  EntryType entryType = EntryType::UPPER_BOUND;

  for (const auto &move : validMoves) {
    if (timeUp)
      break;

    BitBoard tempBoard = bitboard;
    if (tempBoard.makeMove(move.first, move.second, isBlack)) {
      double score = -negamax(tempBoard, depth - 1, -beta, -alpha, !isBlack,
                              startTime, timeLimit, timeUp);

      if (score > bestScore) {
        bestScore = score;
        bestMove = move;
      }

      alpha = std::max(alpha, score);
      if (alpha >= beta) {
        // Beta cutoff
        entryType = EntryType::LOWER_BOUND;
        // history heuristic disabled for now
        break;
      }
    }
  }

  if (bestScore > alpha) {
    entryType = EntryType::EXACT;
  }

  storeTTEntry(hash, bestScore, depth, entryType, bestMove);
  return bestScore;
}

double PandaAIAgent::exactEndgameSearch(
    BitBoard &bitboard, bool isBlack, int emptySquares,
    std::chrono::steady_clock::time_point startTime,
    std::chrono::milliseconds timeLimit, bool &timeUp) {
  nodesSearched++;

  if (nodesSearched % 500 == 0 && isTimeUp(startTime, timeLimit)) {
    timeUp = true;
    return 0.0;
  }

  if (bitboard.isGameOver()) {
    int playerScore = bitboard.getScore(isBlack);
    int opponentScore = bitboard.getScore(!isBlack);
    double finalScore = static_cast<double>(playerScore - opponentScore);

    // Add win/loss bonus for exact search
    if (finalScore > 0) {
      finalScore += 100.0; // Win bonus
    } else if (finalScore < 0) {
      finalScore -= 100.0; // Loss penalty
    }

    return finalScore;
  }

  auto validMoves = bitboard.getValidMoves(isBlack);
  if (validMoves.empty()) {
    if (!bitboard.hasValidMoves(!isBlack)) {
      // Game over
      int playerScore = bitboard.getScore(isBlack);
      int opponentScore = bitboard.getScore(!isBlack);
      double finalScore = static_cast<double>(playerScore - opponentScore);

      if (finalScore > 0) {
        finalScore += 100.0;
      } else if (finalScore < 0) {
        finalScore -= 100.0;
      }

      return finalScore;
    }

    // Pass turn
    return -exactEndgameSearch(bitboard, !isBlack, emptySquares, startTime,
                               timeLimit, timeUp);
  }

  double bestScore = -std::numeric_limits<double>::infinity();

  for (const auto &move : validMoves) {
    if (timeUp)
      break;

    BitBoard tempBoard = bitboard;
    if (tempBoard.makeMove(move.first, move.second, isBlack)) {
      double score = -exactEndgameSearch(tempBoard, !isBlack, emptySquares - 1,
                                         startTime, timeLimit, timeUp);
      bestScore = std::max(bestScore, score);
    }
  }

  return bestScore;
}

inline double PandaAIAgent::evaluatePosition(const BitBoard &bitboard,
                                             bool isBlack) const {
  // EXACT COPY of bitboard agent's evaluation - rely on search advantage
  double score = 0.0;

  uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
  uint64_t opponentBoard = bitboard.getPlayerBoard(!isBlack);
  int totalDiscs = bitboard.getTotalDiscs();

  // Corner control - EXACT 25x weight (same as bitboard)
  uint64_t corners = bitboard.getCornerMask();
  int playerCorners = __builtin_popcountll(playerBoard & corners);
  int opponentCorners = __builtin_popcountll(opponentBoard & corners);
  score += (playerCorners - opponentCorners) * 25.0;

  // Edge control - EXACT 5x weight (same as bitboard)
  uint64_t edges = bitboard.getEdgeMask();
  int playerEdges = __builtin_popcountll(playerBoard & edges);
  int opponentEdges = __builtin_popcountll(opponentBoard & edges);
  score += (playerEdges - opponentEdges) * 5.0;

  // Mobility - EXACT 15x weight (same as bitboard)
  int playerMoves = bitboard.getValidMoves(isBlack).size();
  int opponentMoves = bitboard.getValidMoves(!isBlack).size();
  if (playerMoves + opponentMoves > 0) {
    score += ((double)(playerMoves - opponentMoves) /
              (playerMoves + opponentMoves)) *
             15.0;
  }

  // Disc count - EXACT 5x weight (same as bitboard)
  int playerDiscs = bitboard.getScore(isBlack);
  int opponentDiscs = bitboard.getScore(!isBlack);
  score += ((double)(playerDiscs - opponentDiscs) / totalDiscs) * 5.0;

  // Stability - EXACT 10x weight (same as bitboard)
  uint64_t stableMask = bitboard.getStableMask();
  uint64_t playerStable = playerBoard & stableMask;
  uint64_t opponentStable = opponentBoard & stableMask;
  int playerStableCount = __builtin_popcountll(playerStable);
  int opponentStableCount = __builtin_popcountll(opponentStable);
  score += (playerStableCount - opponentStableCount) * 10.0;

  // EXACT positional evaluation (same as bitboard)
  const auto &table = (totalDiscs < 20)   ? OPENING_VALUES
                      : (totalDiscs > 50) ? ENDGAME_VALUES
                                          : POSITION_VALUES;
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      int cell = bitboard.getCell(r, c);
      if (cell == (isBlack ? 1 : 2)) {
        score += table[r][c];
      } else if (cell == (isBlack ? 2 : 1)) {
        score -= table[r][c];
      }
    }
  }

  return score;
}

PandaAIAgent::EvaluationWeights
PandaAIAgent::getWeights(GamePhase phase) const {
  switch (phase) {
  case GamePhase::OPENING:
    return {20.0, 100.0, 10.0,
            15.0, 1.0,   8.0}; // Prioritize corners, avoid edges
  case GamePhase::MIDGAME:
    return {15.0, 80.0, 20.0, 25.0, 5.0, 10.0}; // Balanced approach
  case GamePhase::ENDGAME:
    return {10.0, 50.0, 15.0, 20.0, 30.0, 5.0}; // Focus on disc count
  }
  return {15.0, 80.0, 20.0, 25.0, 5.0, 10.0}; // Default midgame weights
}

double PandaAIAgent::evaluateMobility(const BitBoard &bitboard,
                                      bool isBlack) const {
  int playerMoves = bitboard.getValidMoves(isBlack).size();
  int opponentMoves = bitboard.getValidMoves(!isBlack).size();

  if (playerMoves + opponentMoves == 0)
    return 0.0;

  return static_cast<double>(playerMoves - opponentMoves) /
         (playerMoves + opponentMoves);
}

double PandaAIAgent::evaluateCornerControl(const BitBoard &bitboard,
                                           bool isBlack) const {
  uint64_t corners = bitboard.getCornerMask();
  uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
  uint64_t opponentBoard = bitboard.getPlayerBoard(!isBlack);

  int playerCorners = __builtin_popcountll(playerBoard & corners);
  int opponentCorners = __builtin_popcountll(opponentBoard & corners);

  return static_cast<double>(playerCorners - opponentCorners);
}

double PandaAIAgent::evaluateEdgeControl(const BitBoard &bitboard,
                                         bool isBlack) const {
  uint64_t edges = bitboard.getEdgeMask();
  uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
  uint64_t opponentBoard = bitboard.getPlayerBoard(!isBlack);

  int playerEdges = __builtin_popcountll(playerBoard & edges);
  int opponentEdges = __builtin_popcountll(opponentBoard & edges);

  return static_cast<double>(playerEdges - opponentEdges);
}

double PandaAIAgent::evaluateStability(const BitBoard &bitboard,
                                       bool isBlack) const {
  // Fast stability approximation using corner and edge control
  double stability = 0.0;

  uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
  uint64_t corners = bitboard.getCornerMask();

  // Corner discs are always stable
  stability += __builtin_popcountll(playerBoard & corners) * 10.0;

  // Edge discs adjacent to corners are more stable
  uint64_t stableEdges = 0;
  if (playerBoard & (1ULL << 0))
    stableEdges |= (1ULL << 1) | (1ULL << 8); // Top-left corner
  if (playerBoard & (1ULL << 7))
    stableEdges |= (1ULL << 6) | (1ULL << 15); // Top-right corner
  if (playerBoard & (1ULL << 56))
    stableEdges |= (1ULL << 48) | (1ULL << 57); // Bottom-left corner
  if (playerBoard & (1ULL << 63))
    stableEdges |= (1ULL << 55) | (1ULL << 62); // Bottom-right corner

  stability += __builtin_popcountll(playerBoard & stableEdges) * 3.0;

  return stability;
}

double PandaAIAgent::evaluateDiscCount(const BitBoard &bitboard,
                                       bool isBlack) const {
  int playerDiscs = bitboard.getScore(isBlack);
  int opponentDiscs = bitboard.getScore(!isBlack);
  int totalDiscs = bitboard.getTotalDiscs();

  if (totalDiscs == 0)
    return 0.0;

  return static_cast<double>(playerDiscs - opponentDiscs) / totalDiscs;
}

double PandaAIAgent::evaluatePositional(const BitBoard &bitboard,
                                        bool isBlack) const {
  GamePhase phase = getGamePhase(bitboard);
  const auto &table = (phase == GamePhase::OPENING)   ? OPENING_VALUES
                      : (phase == GamePhase::ENDGAME) ? ENDGAME_VALUES
                                                      : POSITION_VALUES;

  double score = 0.0;
  uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
  uint64_t opponentBoard = bitboard.getPlayerBoard(!isBlack);

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      uint64_t mask = BitBoard::positionToMask(row, col);
      if (playerBoard & mask) {
        score += table[row][col];
      } else if (opponentBoard & mask) {
        score -= table[row][col];
      }
    }
  }

  return score;
}

bool PandaAIAgent::probeTTEntry(uint64_t hash, int depth, double alpha,
                                double beta, double &score,
                                std::pair<int, int> &bestMove) const {
  size_t index = hash % TT_SIZE;
  const TTEntry &entry = transpositionTable[index];

  if (entry.hash != hash || entry.depth < depth) {
    return false;
  }

  bestMove = entry.bestMove;

  switch (entry.type) {
  case EntryType::EXACT:
    score = entry.score;
    return true;
  case EntryType::LOWER_BOUND:
    if (entry.score >= beta) {
      score = entry.score;
      return true;
    }
    break;
  case EntryType::UPPER_BOUND:
    if (entry.score <= alpha) {
      score = entry.score;
      return true;
    }
    break;
  }

  return false;
}

void PandaAIAgent::storeTTEntry(uint64_t hash, double score, int depth,
                                EntryType type,
                                std::pair<int, int> bestMove) const {
  size_t index = hash % TT_SIZE;
  TTEntry &entry = transpositionTable[index];

  // Always replace strategy (could be improved with depth preference)
  entry.hash = hash;
  entry.score = score;
  entry.depth = depth;
  entry.type = type;
  entry.bestMove = bestMove;
}

std::vector<std::pair<int, int>>
PandaAIAgent::orderMoves(const BitBoard &bitboard,
                         const std::vector<std::pair<int, int>> &moves,
                         bool isBlack, std::pair<int, int> ttMove) const {

  std::vector<std::pair<std::pair<int, int>, double>> scoredMoves;
  scoredMoves.reserve(moves.size());

  for (const auto &move : moves) {
    double score = 0.0;

    // Transposition table move gets highest priority
    if (move == ttMove) {
      score += 10000.0;
    }

    // History heuristic bonus
    int hidx = move.first * 8 + move.second;
    if (hidx >= 0 && hidx < 64) {
      // disabled (tuning): history influence
      // score += static_cast<double>(historyTable[hidx]) * 0.0;
    }

    // Corner moves - EXACT same as bitboard (1000)
    if (isCorner(move.first, move.second)) {
      score += 1000.0;
    }

    // Avoid moves adjacent to empty corners (unless corner is already
    // controlled)
    bool adjacentToEmptyCorner = false;
    for (int dr = -1; dr <= 1; ++dr) {
      for (int dc = -1; dc <= 1; ++dc) {
        int cr = move.first + dr, cc = move.second + dc;
        if (isCorner(cr, cc) && bitboard.getCell(cr, cc) == 0) {
          adjacentToEmptyCorner = true;
          break;
        }
      }
      if (adjacentToEmptyCorner)
        break;
    }

    if (adjacentToEmptyCorner) {
      score -= 500.0; // EXACT same as bitboard
    }

    // Phase-aware fast move evaluation
    int row = move.first, col = move.second;
    int totalDiscs = bitboard.getTotalDiscs();
    bool opening = (totalDiscs <= 20);
    bool endgame = (totalDiscs > 50);

    // Edge moves: de-emphasize in opening to avoid giving minmax easy edges
    if (row == 0 || row == 7 || col == 0 || col == 7) {
      score += opening ? 20.0 : 100.0;
    }

    // Flip count bonus: much smaller in opening to favor quiet,
    // mobility-preserving moves
    BitBoard temp = bitboard;
    int beforeScore = bitboard.getScore(isBlack);
    if (temp.makeMove(row, col, isBlack)) {
      int afterScore = temp.getScore(isBlack);
      int flips = afterScore - beforeScore - 1;
      double flipWeight = opening ? 2.0 : 10.0;
      score += flips * flipWeight;
    }

    // Mobility swing: favor moves that reduce opponent mobility, especially in
    // opening
    BitBoard tmp2 = bitboard;
    if (tmp2.makeMove(row, col, isBlack)) {
      int myMob = tmp2.getValidMoves(isBlack).size();
      int oppMob = tmp2.getValidMoves(!isBlack).size();
      double mobWeight = opening ? 8.0 : 5.0;
      score += (myMob - oppMob) * mobWeight;
    }

    scoredMoves.emplace_back(move, score);
  }

  // Sort moves by score (highest first)
  std::sort(scoredMoves.begin(), scoredMoves.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  std::vector<std::pair<int, int>> orderedMoves;
  orderedMoves.reserve(moves.size());
  for (const auto &scoredMove : scoredMoves) {
    orderedMoves.push_back(scoredMove.first);
  }

  return orderedMoves;
}

double PandaAIAgent::scoreMoveForOrdering(const BitBoard &bitboard,
                                          std::pair<int, int> move,
                                          bool isBlack) const {
  double score = 0.0;

  // Simulate the move to evaluate its impact
  BitBoard tempBoard = bitboard;
  int beforeScore = tempBoard.getScore(isBlack);

  GamePhase phase = getGamePhase(bitboard);

  if (tempBoard.makeMove(move.first, move.second, isBlack)) {
    int afterScore = tempBoard.getScore(isBlack);
    int flippedDiscs = afterScore - beforeScore - 1;

    // Phase-aware flip weighting
    double flipW = (phase == GamePhase::OPENING) ? 2.0 : 10.0;
    score += flippedDiscs * flipW;

    // Mobility impact (stronger in opening against shallow evaluators)
    int playerMobility = tempBoard.getValidMoves(isBlack).size();
    int opponentMobility = tempBoard.getValidMoves(!isBlack).size();
    double mobW = (phase == GamePhase::OPENING) ? 8.0 : 5.0;
    score += (playerMobility - opponentMobility) * mobW;

    // Edge control bonus only outside opening
    if (phase != GamePhase::OPENING && isEdge(move.first, move.second)) {
      score += 20.0;
    }
  }

  return score;
}

bool PandaAIAgent::isOpeningPosition(const BitBoard &bitboard) const {
  return bitboard.getTotalDiscs() <= 16; // Extended opening coverage
}

std::pair<int, int> PandaAIAgent::getOpeningMove(const BitBoard &bitboard,
                                                 bool isBlack) const {
  auto validMoves = bitboard.getValidMoves(isBlack);
  int totalDiscs = bitboard.getTotalDiscs();

  // Opening book based on tournament theory
  if (totalDiscs == 4) {
    // First move - diagonal moves are strongest
    static const std::vector<std::pair<int, int>> firstMoves = {
        {2, 3}, {3, 2}, {4, 5}, {5, 4} // Diagonal moves
    };
    for (const auto &move : firstMoves) {
      if (std::find(validMoves.begin(), validMoves.end(), move) !=
          validMoves.end()) {
        return move;
      }
    }
  } else if (totalDiscs <= 8) {
    // Early opening - prioritize center control
    static const std::vector<std::pair<int, int>> earlyMoves = {
        {2, 2}, {2, 5}, {5, 2}, {5, 5}, // Inner squares (highest priority)
        {2, 3}, {3, 2}, {4, 5}, {5, 4}, // Diagonal continuation
        {2, 4}, {3, 5}, {4, 2}, {5, 3}  // Parallel moves
    };

    return selectBestOpeningMove(bitboard, validMoves, earlyMoves, isBlack);
  } else if (totalDiscs <= 12) {
    // Mid-opening - balance development and safety
    static const std::vector<std::pair<int, int>> midMoves = {
        {1, 1}, {1, 6}, {6, 1}, {6, 6}, // Safe inner positions
        {2, 2}, {2, 5}, {5, 2}, {5, 5}, // Inner squares
        {3, 1}, {1, 3}, {4, 6}, {6, 4}  // Safe edge extensions
    };

    return selectBestOpeningMove(bitboard, validMoves, midMoves, isBlack);
  } else if (totalDiscs <= 16) {
    // Late opening - prepare for midgame transition
    return selectStrategicOpeningMove(bitboard, validMoves, isBlack);
  }

  return {-1, -1}; // No opening book move found
}

std::pair<int, int> PandaAIAgent::selectBestOpeningMove(
    const BitBoard &bitboard,
    const std::vector<std::pair<int, int>> &validMoves,
    const std::vector<std::pair<int, int>> &bookMoves, bool isBlack) const {

  std::vector<std::pair<std::pair<int, int>, double>> scoredMoves;

  for (const auto &move : validMoves) {
    double score = 0.0;

    // Prioritize moves in opening book
    auto it = std::find(bookMoves.begin(), bookMoves.end(), move);
    if (it != bookMoves.end()) {
      score += 200.0 -
               static_cast<double>(std::distance(bookMoves.begin(), it)) * 10.0;
    }

    // Heavily penalize edge moves in opening
    if (isEdge(move.first, move.second)) {
      score -= 150.0;
    }

    // Severely penalize moves adjacent to empty corners
    bool adjacentToEmptyCorner = false;
    for (int dr = -1; dr <= 1; ++dr) {
      for (int dc = -1; dc <= 1; ++dc) {
        int cr = move.first + dr, cc = move.second + dc;
        if (isCorner(cr, cc) && bitboard.getCell(cr, cc) == 0) {
          adjacentToEmptyCorner = true;
          break;
        }
      }
      if (adjacentToEmptyCorner)
        break;
    }

    if (adjacentToEmptyCorner) {
      score -= 300.0;
    }

    // Evaluate position after move
    BitBoard tempBoard = bitboard;
    if (tempBoard.makeMove(move.first, move.second, isBlack)) {
      // Mobility differential
      int playerMoves = tempBoard.getValidMoves(isBlack).size();
      int opponentMoves = tempBoard.getValidMoves(!isBlack).size();
      score += (playerMoves - opponentMoves) * 20.0;

      // Prefer central positions
      double centerDistance =
          std::abs(move.first - 3.5) + std::abs(move.second - 3.5);
      score += (7.0 - centerDistance) * 5.0;
    }

    scoredMoves.emplace_back(move, score);
  }

  if (!scoredMoves.empty()) {
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });
    return scoredMoves[0].first;
  }

  return {-1, -1};
}

std::pair<int, int> PandaAIAgent::selectStrategicOpeningMove(
    const BitBoard &bitboard,
    const std::vector<std::pair<int, int>> &validMoves, bool isBlack) const {

  // Use simplified evaluation for late opening moves
  std::vector<std::pair<std::pair<int, int>, double>> scoredMoves;

  for (const auto &move : validMoves) {
    double score = scoreMoveForOrdering(bitboard, move, isBlack);

    // Corner moves are always good
    if (isCorner(move.first, move.second)) {
      score += 500.0;
    }

    scoredMoves.emplace_back(move, score);
  }

  if (!scoredMoves.empty()) {
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });
    return scoredMoves[0].first;
  }

  return {-1, -1};
}

REGISTER_AI_AGENT(PandaAIAgent, "panda")
