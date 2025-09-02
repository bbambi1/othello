#pragma once

#include "ai_agent_base.h"
#include "bitboard.h"
#include <chrono>
#include <unordered_map>
#include <limits>
#include <random>
#include <vector>
#include <algorithm>

class Board;
enum class CellState;

class BitBoardAIAgent : public AIAgentBase {
public:
    BitBoardAIAgent(const std::string& name = "BitBoard",
                    const std::string& author = "Enhanced AI",
                    int maxDepth = 8);

    // Determine the best move for `player` on the given board within a
    // specified time limit.  Returns {row,col} or {-1,-1} if no move.
    std::pair<int,int> getBestMove(const Board& board, CellState player,
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;

private:
    int maxDepth;

    // Min/max search with alpha-beta pruning.  Returns an evaluation
    // score for the subtree rooted at the current position.
    double bitboardMinMax(BitBoard& bitboard, int depth, double alpha, double beta,
                          bool isBlack, bool isMaximizing,
                          std::chrono::steady_clock::time_point startTime,
                          std::chrono::milliseconds timeLimit);

    // Overall evaluation of a position: combines several heuristics.
    double evaluateBitboard(const BitBoard& bitboard, bool isBlack) const;
    // Individual heuristics
    double evaluateCornerControlBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateEdgeControlBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateMobilityBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateStabilityBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateDiscCountBitboard(const BitBoard& bitboard, bool isBlack) const;

    // Positional value tables for early, mid and late game
    static const std::array<std::array<int,8>,8> POSITION_VALUES;
    static const std::array<std::array<int,8>,8> EARLY_GAME_VALUES;
    static const std::array<std::array<int,8>,8> LATE_GAME_VALUES;

    // Transposition table support
    enum class EntryType { EXACT, LOWER_BOUND, UPPER_BOUND };
    struct TranspositionEntry {
        uint64_t hash;
        double score;
        int depth;
        EntryType type;
        std::chrono::steady_clock::time_point timestamp;
        TranspositionEntry() : hash(0), score(0.0), depth(0), type(EntryType::EXACT) {}
        TranspositionEntry(uint64_t h, double s, int d, EntryType t)
            : hash(h), score(s), depth(d), type(t) {}
    };
    mutable std::unordered_map<uint64_t,TranspositionEntry> transpositionTable;
    static constexpr size_t MAX_TRANSPOSITION_SIZE = 1000000;

    // Hashing helper
    uint64_t getZobristHash(const BitBoard& bitboard) const;
    // Clear or query table
    void clearTranspositionTable();
    size_t getTranspositionTableSize() const;

    // Player utility
    bool isBlackPlayer(CellState player) const;
    CellState playerFromBool(bool isBlack) const;

    // Move ordering: sorts moves from most promising to least.
    std::vector<std::pair<int,int>> orderMoves(const BitBoard& bitboard,
                                              const std::vector<std::pair<int,int>>& moves,
                                              bool isBlack) const;

    // Quiescence search placeholder (not implemented here)
    double quiescenceSearch(BitBoard& bitboard, double alpha, double beta,
                            bool isBlack, std::chrono::steady_clock::time_point startTime,
                            std::chrono::milliseconds timeLimit) const { return 0.0; }
};
