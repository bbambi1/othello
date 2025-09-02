#include "bitboard_ai_agent.h"
#include <algorithm>
#include <limits>
#include <chrono>
#include <random>

// Position value tables for different game phases
const std::array<std::array<int, 8>, 8> BitBoardAIAgent::POSITION_VALUES = {{
    {{100, -20, 10, 5, 5, 10, -20, 100}},
    {{-20, -50, -2, -2, -2, -2, -50, -20}},
    {{10, -2, -1, -1, -1, -1, -2, 10}},
    {{5, -2, -1, -1, -1, -1, -2, 5}},
    {{5, -2, -1, -1, -1, -1, -2, 5}},
    {{10, -2, -1, -1, -1, -1, -2, 10}},
    {{-20, -50, -2, -2, -2, -2, -50, -20}},
    {{100, -20, 10, 5, 5, 10, -20, 100}}
}};

const std::array<std::array<int, 8>, 8> BitBoardAIAgent::EARLY_GAME_VALUES = {{
    {{100, -20, 10, 5, 5, 10, -20, 100}},
    {{-20, -50, -2, -2, -2, -2, -50, -20}},
    {{10, -2, -1, -1, -1, -1, -2, 10}},
    {{5, -2, -1, -1, -1, -1, -2, 5}},
    {{5, -2, -1, -1, -1, -1, -2, 5}},
    {{10, -2, -1, -1, -1, -1, -2, 10}},
    {{-20, -50, -2, -2, -2, -2, -50, -20}},
    {{100, -20, 10, 5, 5, 10, -20, 100}}
}};

const std::array<std::array<int, 8>, 8> BitBoardAIAgent::LATE_GAME_VALUES = {{
    {{100, 50, 30, 20, 20, 30, 50, 100}},
    {{50, 30, 20, 10, 10, 20, 30, 50}},
    {{30, 20, 10, 5, 5, 10, 20, 30}},
    {{20, 10, 5, 0, 0, 5, 10, 20}},
    {{20, 10, 5, 0, 0, 5, 10, 20}},
    {{30, 20, 10, 5, 5, 10, 20, 30}},
    {{50, 30, 20, 10, 10, 20, 30, 50}},
    {{100, 50, 30, 20, 20, 30, 50, 100}}
}};

BitBoardAIAgent::BitBoardAIAgent(const std::string& name, const std::string& author, int depth) 
    : AIAgentBase(name, author), maxDepth(depth) {
}

std::pair<int, int> BitBoardAIAgent::getBestMove(const Board& board, CellState player, 
                                               std::chrono::milliseconds timeLimit) {
    auto startTime = std::chrono::steady_clock::now();
    
    // Convert Board to BitBoard
    BitBoard bitboard;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            CellState cellState = board.getCell(row, col);
            if (cellState == CellState::BLACK) {
                bitboard.setCell(row, col, 1);
            } else if (cellState == CellState::WHITE) {
                bitboard.setCell(row, col, 2);
            }
        }
    }
    
    bool isBlack = (player == CellState::BLACK);
    auto validMoves = bitboard.getValidMoves(isBlack);
    
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    // Order moves for better alpha-beta pruning
    validMoves = orderMoves(bitboard, validMoves, isBlack);
    
    std::pair<int, int> bestMove = validMoves[0];
    double bestScore = std::numeric_limits<double>::lowest();
    
    for (const auto& move : validMoves) {
        // Check time before processing each move
        if (isTimeUp(startTime, timeLimit)) {
            break;
        }
        
        BitBoard tempBitboard = bitboard;
        if (tempBitboard.makeMove(move.first, move.second, isBlack)) {
            double score = bitboardMinMax(tempBitboard, maxDepth - 1, 
                                        std::numeric_limits<double>::lowest(),
                                        std::numeric_limits<double>::max(),
                                        isBlack, false, startTime, timeLimit);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }
    
    return bestMove;
}

double BitBoardAIAgent::bitboardMinMax(BitBoard& bitboard, int depth, double alpha, double beta, 
                                     bool isBlack, bool isMaximizing, 
                                     std::chrono::steady_clock::time_point startTime,
                                     std::chrono::milliseconds timeLimit) {
    // Check time at the beginning of each recursive call
    if (isTimeUp(startTime, timeLimit)) {
        return 0.0;
    }
    
    // Check transposition table with Zobrist hashing
    uint64_t hash = getZobristHash(bitboard);
    auto it = transpositionTable.find(hash);
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        const auto& entry = it->second;
        switch (entry.type) {
            case EntryType::EXACT:
                return entry.score;
            case EntryType::LOWER_BOUND:
                if (entry.score >= beta) return entry.score;
                break;
            case EntryType::UPPER_BOUND:
                if (entry.score <= alpha) return entry.score;
                break;
        }
    }
    
    if (depth == 0 || bitboard.isGameOver()) {
        double score = evaluateBitboard(bitboard, isBlack);
        
        // Store in transposition table
        if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
            transpositionTable[hash] = TranspositionEntry(hash, score, depth, EntryType::EXACT);
        }
        
        return score;
    }
    
    auto validMoves = bitboard.getValidMoves(isMaximizing ? isBlack : !isBlack);
    if (validMoves.empty()) {
        // No moves available, pass turn
        double score = bitboardMinMax(bitboard, depth - 1, alpha, beta, 
                                    isBlack, !isMaximizing, startTime, timeLimit);
        return score;
    }
    
    // Order moves for better alpha-beta pruning
    validMoves = orderMoves(bitboard, validMoves, isMaximizing ? isBlack : !isBlack);
    
    if (isMaximizing) {
        double maxScore = std::numeric_limits<double>::lowest();
        EntryType entryType = EntryType::UPPER_BOUND;
        
        for (const auto& move : validMoves) {
            if (isTimeUp(startTime, timeLimit)) {
                break;
            }
            
            BitBoard tempBitboard = bitboard;
            if (tempBitboard.makeMove(move.first, move.second, isBlack)) {
                double score = bitboardMinMax(tempBitboard, depth - 1, alpha, beta, 
                                            isBlack, false, startTime, timeLimit);
                maxScore = std::max(maxScore, score);
                alpha = std::max(alpha, score);
                if (beta <= alpha) {
                    // Beta cutoff - store as upper bound
                    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
                        transpositionTable[hash] = TranspositionEntry(hash, maxScore, depth, EntryType::UPPER_BOUND);
                    }
                    break; // Alpha-beta pruning
                }
            }
        }
        
        // Determine entry type based on final score
        if (maxScore <= alpha) {
            entryType = EntryType::UPPER_BOUND;
        } else if (maxScore >= beta) {
            entryType = EntryType::LOWER_BOUND;
        } else {
            entryType = EntryType::EXACT;
        }
        
        // Store in transposition table
        if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
            transpositionTable[hash] = TranspositionEntry(hash, maxScore, depth, entryType);
        }
        
        return maxScore;
    } else {
        double minScore = std::numeric_limits<double>::max();
        EntryType entryType = EntryType::LOWER_BOUND;
        
        for (const auto& move : validMoves) {
            if (isTimeUp(startTime, timeLimit)) {
                break;
            }
            
            BitBoard tempBitboard = bitboard;
            if (tempBitboard.makeMove(move.first, move.second, !isBlack)) {
                double score = bitboardMinMax(tempBitboard, depth - 1, alpha, beta, 
                                            isBlack, true, startTime, timeLimit);
                minScore = std::min(minScore, score);
                beta = std::min(beta, score);
                if (beta <= alpha) {
                    // Alpha cutoff - store as lower bound
                    if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
                        transpositionTable[hash] = TranspositionEntry(hash, minScore, depth, EntryType::LOWER_BOUND);
                    }
                    break; // Alpha-beta pruning
                }
            }
        }
        
        // Determine entry type based on final score
        if (minScore <= alpha) {
            entryType = EntryType::UPPER_BOUND;
        } else if (minScore >= beta) {
            entryType = EntryType::LOWER_BOUND;
        } else {
            entryType = EntryType::EXACT;
        }
        
        // Store in transposition table
        if (transpositionTable.size() < MAX_TRANSPOSITION_SIZE) {
            transpositionTable[hash] = TranspositionEntry(hash, minScore, depth, entryType);
        }
        
        return minScore;
    }
}

double BitBoardAIAgent::evaluateBitboard(const BitBoard& bitboard, bool isBlack) const {
    double score = 0.0;
    
    // Corner control with high weight
    score += evaluateCornerControlBitboard(bitboard, isBlack) * 25.0;
    
    // Edge control
    score += evaluateEdgeControlBitboard(bitboard, isBlack) * 5.0;
    
    // Mobility (ability to make moves)
    score += evaluateMobilityBitboard(bitboard, isBlack) * 15.0;
    
    // Disc count
    score += evaluateDiscCountBitboard(bitboard, isBlack) * 5.0;
    
    // Stability
    score += evaluateStabilityBitboard(bitboard, isBlack) * 10.0;
    
    // Position values based on game phase
    int totalDiscs = bitboard.getTotalDiscs();
    const auto& positionValues = (totalDiscs < 20) ? EARLY_GAME_VALUES : 
                                (totalDiscs > 50) ? LATE_GAME_VALUES : POSITION_VALUES;
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int cellState = bitboard.getCell(row, col);
            if (cellState == (isBlack ? 1 : 2)) {
                score += positionValues[row][col];
            } else if (cellState == (isBlack ? 2 : 1)) {
                score -= positionValues[row][col];
            }
        }
    }
    
    return score;
}

double BitBoardAIAgent::evaluateCornerControlBitboard(const BitBoard& bitboard, bool isBlack) const {
    uint64_t corners = bitboard.getCornerMask();
    uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
    uint64_t opponentBoard = bitboard.getOpponentBoard(isBlack);
    
    int playerCorners = __builtin_popcountll(playerBoard & corners);
    int opponentCorners = __builtin_popcountll(opponentBoard & corners);
    
    return static_cast<double>(playerCorners - opponentCorners);
}

double BitBoardAIAgent::evaluateEdgeControlBitboard(const BitBoard& bitboard, bool isBlack) const {
    uint64_t edges = bitboard.getEdgeMask();
    uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
    uint64_t opponentBoard = bitboard.getOpponentBoard(isBlack);
    
    int playerEdges = __builtin_popcountll(playerBoard & edges);
    int opponentEdges = __builtin_popcountll(opponentBoard & edges);
    
    return static_cast<double>(playerEdges - opponentEdges);
}

double BitBoardAIAgent::evaluateMobilityBitboard(const BitBoard& bitboard, bool isBlack) const {
    int playerMoves = bitboard.getValidMoves(isBlack).size();
    int opponentMoves = bitboard.getValidMoves(!isBlack).size();
    
    if (playerMoves + opponentMoves == 0) return 0.0;
    
    return static_cast<double>(playerMoves - opponentMoves) / (playerMoves + opponentMoves);
}

double BitBoardAIAgent::evaluateStabilityBitboard(const BitBoard& bitboard, bool isBlack) const {
    // Simple stability evaluation based on corner adjacency
    double score = 0.0;
    uint64_t playerBoard = bitboard.getPlayerBoard(isBlack);
    
    // Check each player disc for stability
    uint64_t temp = playerBoard;
    while (temp) {
        int bit = __builtin_ctzll(temp);
        auto [row, col] = BitBoard::bitToPosition(bit);
        
        // Check if disc is adjacent to corners (less stable)
        bool adjacentToCorner = false;
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                int nr = row + dr;
                int nc = col + dc;
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    if ((nr == 0 || nr == 7) && (nc == 0 || nc == 7)) {
                        adjacentToCorner = true;
                        break;
                    }
                }
            }
            if (adjacentToCorner) break;
        }
        
        if (adjacentToCorner) {
            score -= 2.0; // Less stable
        } else {
            score += 1.0; // More stable
        }
        
        temp &= temp - 1; // Clear the lowest set bit
    }
    
    return score;
}

double BitBoardAIAgent::evaluateDiscCountBitboard(const BitBoard& bitboard, bool isBlack) const {
    int playerDiscs = bitboard.getScore(isBlack);
    int opponentDiscs = bitboard.getScore(!isBlack);
    int totalDiscs = bitboard.getTotalDiscs();
    
    if (totalDiscs == 0) return 0.0;
    
    return static_cast<double>(playerDiscs - opponentDiscs) / totalDiscs;
}

uint64_t BitBoardAIAgent::getZobristHash(const BitBoard& bitboard) const {
    return bitboard.getZobristHash();
}

void BitBoardAIAgent::clearTranspositionTable() {
    transpositionTable.clear();
}

size_t BitBoardAIAgent::getTranspositionTableSize() const {
    return transpositionTable.size();
}

bool BitBoardAIAgent::isBlackPlayer(CellState player) const {
    return player == CellState::BLACK;
}

CellState BitBoardAIAgent::playerFromBool(bool isBlack) const {
    return isBlack ? CellState::BLACK : CellState::WHITE;
}

std::vector<std::pair<int, int>> BitBoardAIAgent::orderMoves(const BitBoard& bitboard, 
                                                           const std::vector<std::pair<int, int>>& moves, 
                                                           bool isBlack) const {
    std::vector<std::pair<std::pair<int, int>, double>> moveScores;
    
    for (const auto& move : moves) {
        double score = 0.0;
        
        // Prioritize corners
        if ((move.first == 0 || move.first == 7) && (move.second == 0 || move.second == 7)) {
            score += 1000.0;
        }
        // Avoid corner-adjacent positions
        else if (((move.first == 0 || move.first == 7) && (move.second == 1 || move.second == 6)) ||
                 ((move.first == 1 || move.first == 6) && (move.second == 0 || move.second == 7))) {
            score -= 500.0;
        }
        // Prefer edges
        else if (move.first == 0 || move.first == 7 || move.second == 0 || move.second == 7) {
            score += 100.0;
        }
        
        // Count potential flips
        BitBoard tempBitboard = bitboard;
        if (tempBitboard.makeMove(move.first, move.second, isBlack)) {
            int flips = __builtin_popcountll(tempBitboard.getFlippedBitboard(move.first, move.second, isBlack));
            score += flips * 10.0;
        }
        
        moveScores.emplace_back(move, score);
    }
    
    // Sort by score (descending)
    std::sort(moveScores.begin(), moveScores.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::pair<int, int>> orderedMoves;
    for (const auto& [move, score] : moveScores) {
        orderedMoves.push_back(move);
    }
    
    return orderedMoves;
}

// Register the AI agent
REGISTER_AI_AGENT(BitBoardAIAgent, "bitboard")
