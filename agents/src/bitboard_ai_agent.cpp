#include "bitboard_ai_agent.h"
#include <algorithm>
#include <limits>
#include <chrono>
#include <random>

const std::array<std::array<int,8>,8> BitBoardAIAgent::POSITION_VALUES = {{
    {{100, -20, 10, 5, 5, 10, -20, 100}},
    {{-20, -50, -2, -2, -2, -2, -50, -20}},
    {{10, -2, -1, -1, -1, -1, -2, 10}},
    {{5, -2, -1, -1, -1, -1, -2, 5}},
    {{5, -2, -1, -1, -1, -1, -2, 5}},
    {{10, -2, -1, -1, -1, -1, -2, 10}},
    {{-20, -50, -2, -2, -2, -2, -50, -20}},
    {{100, -20, 10, 5, 5, 10, -20, 100}}
}};

const std::array<std::array<int,8>,8> BitBoardAIAgent::EARLY_GAME_VALUES = POSITION_VALUES;

const std::array<std::array<int,8>,8> BitBoardAIAgent::LATE_GAME_VALUES = {{
    {{100, 50, 30, 20, 20, 30, 50, 100}},
    {{50, 30, 20, 10, 10, 20, 30, 50}},
    {{30, 20, 10, 5, 5, 10, 20, 30}},
    {{20, 10, 5, 0, 0, 5, 10, 20}},
    {{20, 10, 5, 0, 0, 5, 10, 20}},
    {{30, 20, 10, 5, 5, 10, 20, 30}},
    {{50, 30, 20, 10, 10, 20, 30, 50}},
    {{100, 50, 30, 20, 20, 30, 50, 100}}
}};

BitBoardAIAgent::BitBoardAIAgent(const std::string& name,
                                 const std::string& author,
                                 int depth)
    : AIAgentBase(name, author), maxDepth(depth) {}

inline std::pair<int,int> BitBoardAIAgent::getBestMove(const Board& board, CellState player,
                                                       std::chrono::milliseconds timeLimit) {
    auto startTime = std::chrono::steady_clock::now();
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
    if (moves.empty()) return {-1, -1};
    moves = orderMoves(bitboard, moves, isBlackTurn);
    std::pair<int,int> bestMove = moves.front();
    double bestScore = std::numeric_limits<double>::lowest();
    for (const auto& mv : moves) {
        if (isTimeUp(startTime, timeLimit)) break;
        BitBoard temp = bitboard;
        if (temp.makeMove(mv.first, mv.second, isBlackTurn)) {
            double score = bitboardMinMax(temp, maxDepth - 1,
                                         std::numeric_limits<double>::lowest(),
                                         std::numeric_limits<double>::max(),
                                         isBlackTurn, false,
                                         startTime, timeLimit);
            if (score > bestScore) {
                bestScore = score;
                bestMove = mv;
            }
        }
    }
    return bestMove;
}

inline double BitBoardAIAgent::bitboardMinMax(BitBoard& bitboard, int depth,
                                             double alpha, double beta,
                                             bool isBlack, bool isMaximizing,
                                             std::chrono::steady_clock::time_point startTime,
                                             std::chrono::milliseconds timeLimit) {
    if (isTimeUp(startTime, timeLimit)) return 0.0;
    uint64_t hash = getZobristHash(bitboard);
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        const auto& entry = it->second;
        switch (entry.type) {
            case EntryType::EXACT:      return entry.score;
            case EntryType::LOWER_BOUND: if (entry.score >= beta) return entry.score; break;
            case EntryType::UPPER_BOUND: if (entry.score <= alpha) return entry.score; break;
        }
    }
    if (depth == 0 || bitboard.isGameOver()) {
        double sc = evaluateBitboard(bitboard, isBlack);
        if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
            transpositionTable[hash] = TranspositionEntry(hash, sc, depth, EntryType::EXACT);
        }
        return sc;
    }
    auto moves = bitboard.getValidMoves(isMaximizing ? isBlack : !isBlack);
    if (moves.empty()) {
        double sc = bitboardMinMax(bitboard, depth - 1, alpha, beta,
                                   isBlack, !isMaximizing, startTime, timeLimit);
        return sc;
    }
    moves = orderMoves(bitboard, moves, isMaximizing ? isBlack : !isBlack);
    if (isMaximizing) {
        double best = std::numeric_limits<double>::lowest();
        EntryType eType = EntryType::UPPER_BOUND;
        for (const auto& mv : moves) {
            if (isTimeUp(startTime, timeLimit)) break;
            BitBoard temp = bitboard;
            if (temp.makeMove(mv.first, mv.second, isBlack)) {
                double sc = bitboardMinMax(temp, depth - 1, alpha, beta,
                                            isBlack, false, startTime, timeLimit);
                best = std::max(best, sc);
                alpha = std::max(alpha, sc);
                if (beta <= alpha) {
                    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
                        transpositionTable[hash] = TranspositionEntry(hash, best, depth, EntryType::UPPER_BOUND);
                    }
                    break;
                }
            }
        }
        if (best <= alpha) eType = EntryType::UPPER_BOUND;
        else if (best >= beta) eType = EntryType::LOWER_BOUND;
        else eType = EntryType::EXACT;
        if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
            transpositionTable[hash] = TranspositionEntry(hash, best, depth, eType);
        }
        return best;
    } else {
        double best = std::numeric_limits<double>::max();
        EntryType eType = EntryType::LOWER_BOUND;
        for (const auto& mv : moves) {
            if (isTimeUp(startTime, timeLimit)) break;
            BitBoard temp = bitboard;
            if (temp.makeMove(mv.first, mv.second, !isBlack)) {
                double sc = bitboardMinMax(temp, depth - 1, alpha, beta,
                                            isBlack, true, startTime, timeLimit);
                best = std::min(best, sc);
                beta = std::min(beta, sc);
                if (beta <= alpha) {
                    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
                        transpositionTable[hash] = TranspositionEntry(hash, best, depth, EntryType::LOWER_BOUND);
                    }
                    break;
                }
            }
        }
        if (best <= alpha) eType = EntryType::UPPER_BOUND;
        else if (best >= beta) eType = EntryType::LOWER_BOUND;
        else eType = EntryType::EXACT;
        if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
            transpositionTable[hash] = TranspositionEntry(hash, best, depth, eType);
        }
        return best;
    }
}

inline double BitBoardAIAgent::evaluateBitboard(const BitBoard& bitboard, bool isBlack) const {
    double sc = 0.0;
    sc += evaluateCornerControlBitboard(bitboard, isBlack) * 25.0;
    sc += evaluateEdgeControlBitboard(bitboard, isBlack) * 5.0;
    sc += evaluateMobilityBitboard(bitboard, isBlack) * 15.0;
    sc += evaluateDiscCountBitboard(bitboard, isBlack) * 5.0;
    sc += evaluateStabilityBitboard(bitboard, isBlack) * 10.0;
    int discs = bitboard.getTotalDiscs();
    const auto& table = (discs < 20) ? EARLY_GAME_VALUES : (discs > 50) ? LATE_GAME_VALUES : POSITION_VALUES;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int st = bitboard.getCell(r, c);
            if (st == (isBlack ? 1 : 2)) sc += table[r][c];
            else if (st == (isBlack ? 2 : 1)) sc -= table[r][c];
        }
    }
    return sc;
}

inline double BitBoardAIAgent::evaluateCornerControlBitboard(const BitBoard& bb, bool isBlack) const {
    uint64_t corners = bb.getCornerMask();
    uint64_t player = bb.getPlayerBoard(isBlack);
    uint64_t opponent = bb.getOpponentBoard(isBlack);
    int pc = __builtin_popcountll(player & corners);
    int oc = __builtin_popcountll(opponent & corners);
    return static_cast<double>(pc - oc);
}

inline double BitBoardAIAgent::evaluateEdgeControlBitboard(const BitBoard& bb, bool isBlack) const {
    uint64_t edges = bb.getEdgeMask();
    uint64_t player = bb.getPlayerBoard(isBlack);
    uint64_t opponent = bb.getOpponentBoard(isBlack);
    int pe = __builtin_popcountll(player & edges);
    int oe = __builtin_popcountll(opponent & edges);
    return static_cast<double>(pe - oe);
}

inline double BitBoardAIAgent::evaluateMobilityBitboard(const BitBoard& bb, bool isBlack) const {
    int pMoves = bb.getValidMoves(isBlack).size();
    int oMoves = bb.getValidMoves(!isBlack).size();
    if (pMoves + oMoves == 0) return 0.0;
    return static_cast<double>(pMoves - oMoves) / (pMoves + oMoves);
}

inline double BitBoardAIAgent::evaluateStabilityBitboard(const BitBoard& bb, bool isBlack) const {
    double sc = 0.0;
    uint64_t player = bb.getPlayerBoard(isBlack);
    uint64_t tmp = player;
    while (tmp) {
        int bit = __builtin_ctzll(tmp);
        tmp &= tmp - 1;
        auto [r, c] = BitBoard::bitToPosition(bit);
        bool adjacentCorner = false;
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int nr = r + dr;
                int nc = c + dc;
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    if ((nr == 0 || nr == 7) && (nc == 0 || nc == 7)) {
                        adjacentCorner = true;
                        break;
                    }
                }
            }
            if (adjacentCorner) break;
        }
        sc += adjacentCorner ? -2.0 : 1.0;
    }
    return sc;
}

inline double BitBoardAIAgent::evaluateDiscCountBitboard(const BitBoard& bb, bool isBlack) const {
    int p = bb.getScore(isBlack);
    int o = bb.getScore(!isBlack);
    int t = bb.getTotalDiscs();
    if (t == 0) return 0.0;
    return static_cast<double>(p - o) / t;
}

inline uint64_t BitBoardAIAgent::getZobristHash(const BitBoard& bb) const {
    return bb.getZobristHash();
}

inline void BitBoardAIAgent::clearTranspositionTable() {
    transpositionTable.clear();
}

inline size_t BitBoardAIAgent::getTranspositionTableSize() const {
    return transpositionTable.size();
}

inline bool BitBoardAIAgent::isBlackPlayer(CellState p) const {
    return p == CellState::BLACK;
}

inline CellState BitBoardAIAgent::playerFromBool(bool isBlack) const {
    return isBlack ? CellState::BLACK : CellState::WHITE;
}

inline std::vector<std::pair<int,int>> BitBoardAIAgent::orderMoves(const BitBoard& bb,
                                                                   const std::vector<std::pair<int,int>>& moves,
                                                                   bool isBlack) const {
    std::vector<std::pair<std::pair<int,int>,double>> moveScores;
    for (const auto& mv : moves) {
        double score = 0.0;
        if ((mv.first == 0 || mv.first == 7) && (mv.second == 0 || mv.second == 7)) score += 1000.0;
        else if (((mv.first == 0 || mv.first == 7) && (mv.second == 1 || mv.second == 6)) ||
                 ((mv.first == 1 || mv.first == 6) && (mv.second == 0 || mv.second == 7))) score -= 500.0;
        else if (mv.first == 0 || mv.first == 7 || mv.second == 0 || mv.second == 7) score += 100.0;
        BitBoard tmp = bb;
        int before = bb.getScore(isBlack);
        if (tmp.makeMove(mv.first, mv.second, isBlack)) {
            int after = tmp.getScore(isBlack);
            int flips = after - before - 1;
            if (flips > 0) score += flips * 10.0;
        }
        moveScores.emplace_back(mv, score);
    }
    std::sort(moveScores.begin(), moveScores.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    std::vector<std::pair<int,int>> ordered;
    ordered.reserve(moveScores.size());
    for (const auto& ms : moveScores) ordered.push_back(ms.first);
    return ordered;
}

REGISTER_AI_AGENT(BitBoardAIAgent, "bitboard")
