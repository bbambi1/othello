#pragma once

#include "ai_agent_base.h"
#include "bitboard.h"
#include <chrono>
#include <unordered_map>

// BitBoard AI agent with optimized min/max and alpha-beta pruning
// Uses bitboards for extremely fast move generation and evaluation
class BitBoardAIAgent : public AIAgentBase {
public:
    BitBoardAIAgent(const std::string& name = "BitBoard", 
                   const std::string& author = "Enhanced AI", 
                   int maxDepth = 8);
    
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
    
private:
    int maxDepth;
    
    // Bitboard-based min/max with alpha-beta pruning
    double bitboardMinMax(BitBoard& bitboard, int depth, double alpha, double beta, 
                         bool isBlack, bool isMaximizing, 
                         std::chrono::steady_clock::time_point startTime,
                         std::chrono::milliseconds timeLimit);
    
    // Optimized evaluation using bitboard operations
    double evaluateBitboard(const BitBoard& bitboard, bool isBlack) const;
    
    // Bitboard-specific evaluation functions
    double evaluateCornerControlBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateEdgeControlBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateMobilityBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateStabilityBitboard(const BitBoard& bitboard, bool isBlack) const;
    double evaluateDiscCountBitboard(const BitBoard& bitboard, bool isBlack) const;
    
    // Position value tables for different game phases
    static const std::array<std::array<int, 8>, 8> POSITION_VALUES;
    static const std::array<std::array<int, 8>, 8> EARLY_GAME_VALUES;
    static const std::array<std::array<int, 8>, 8> LATE_GAME_VALUES;
    
    // Transposition table for caching evaluations
    enum class EntryType {
        EXACT,      // Exact score
        LOWER_BOUND, // Score is lower bound (alpha)
        UPPER_BOUND  // Score is upper bound (beta)
    };
    
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
    
    mutable std::unordered_map<uint64_t, TranspositionEntry> transpositionTable;
    static constexpr size_t MAX_TRANSPOSITION_SIZE = 1000000;
    
    // Zobrist hashing for bitboard positions
    uint64_t getZobristHash(const BitBoard& bitboard) const;
    
    // Transposition table management
    void clearTranspositionTable();
    size_t getTranspositionTableSize() const;
    
    // Utility functions
    bool isBlackPlayer(CellState player) const;
    CellState playerFromBool(bool isBlack) const;
    
    // Move ordering for better alpha-beta pruning
    std::vector<std::pair<int, int>> orderMoves(const BitBoard& bitboard, 
                                               const std::vector<std::pair<int, int>>& moves, 
                                               bool isBlack) const;
    
    // Quiescence search for better endgame evaluation
    double quiescenceSearch(BitBoard& bitboard, double alpha, double beta, 
                           bool isBlack, std::chrono::steady_clock::time_point startTime,
                           std::chrono::milliseconds timeLimit) const;
};
