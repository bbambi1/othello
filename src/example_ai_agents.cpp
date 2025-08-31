#include "example_ai_agents.h"
#include <random>
#include <algorithm>
#include <chrono>

// Random AI Agent Implementation
RandomAIAgent::RandomAIAgent(const std::string& name) : AIAgentBase(name) {
}

std::pair<int, int> RandomAIAgent::getBestMove(const Board& board, CellState player) {
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    // Use current time as seed for randomness
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, validMoves.size() - 1);
    
    return validMoves[dis(gen)];
}

// Greedy AI Agent Implementation
GreedyAIAgent::GreedyAIAgent(const std::string& name) : AIAgentBase(name) {
}

std::pair<int, int> GreedyAIAgent::getBestMove(const Board& board, CellState player) {
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    std::pair<int, int> bestMove = validMoves[0];
    int maxFlips = 0;
    
    for (const auto& move : validMoves) {
        auto flippedDiscs = board.getFlippedDiscs(move.first, move.second, player);
        if (flippedDiscs.size() > maxFlips) {
            maxFlips = flippedDiscs.size();
            bestMove = move;
        }
    }
    
    return bestMove;
}

// MinMax AI Agent Implementation
MinMaxAIAgent::MinMaxAIAgent(const std::string& name, int depth) 
    : AIAgentBase(name), maxDepth(depth) {
}

std::pair<int, int> MinMaxAIAgent::getBestMove(const Board& board, CellState player) {
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    std::pair<int, int> bestMove = validMoves[0];
    double bestScore = std::numeric_limits<double>::lowest();
    
    for (const auto& move : validMoves) {
        Board tempBoard = board;
        if (tempBoard.makeMove(move.first, move.second, player)) {
            double score = minMax(tempBoard, maxDepth - 1, 
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                player, false);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }
    
    return bestMove;
}

double MinMaxAIAgent::minMax(Board& board, int depth, double alpha, double beta, 
                            CellState player, bool isMaximizing) {
    if (depth == 0 || board.isGameOver()) {
        return evaluateBoard(board, player);
    }
    
    auto validMoves = getValidMoves(board, isMaximizing ? player : getOpponent(player));
    if (validMoves.empty()) {
        return evaluateBoard(board, player);
    }
    
    if (isMaximizing) {
        double maxScore = std::numeric_limits<double>::lowest();
        for (const auto& move : validMoves) {
            Board tempBoard = board;
            if (tempBoard.makeMove(move.first, move.second, player)) {
                double score = minMax(tempBoard, depth - 1, alpha, beta, player, false);
                maxScore = std::max(maxScore, score);
                alpha = std::max(alpha, score);
                if (beta <= alpha) break; // Alpha-beta pruning
            }
        }
        return maxScore;
    } else {
        double minScore = std::numeric_limits<double>::max();
        for (const auto& move : validMoves) {
            Board tempBoard = board;
            if (tempBoard.makeMove(move.first, move.second, getOpponent(player))) {
                double score = minMax(tempBoard, depth - 1, alpha, beta, player, true);
                minScore = std::min(minScore, score);
                beta = std::min(beta, score);
                if (beta <= alpha) break; // Alpha-beta pruning
            }
        }
        return minScore;
    }
}

double MinMaxAIAgent::evaluateBoard(const Board& board, CellState player) const {
    double score = 0.0;
    
    // Corner control
    score += evaluateCornerControl(board, player);
    
    // Edge control
    score += evaluateEdgeControl(board, player);
    
    // Mobility
    score += evaluateMobility(board, player) * 10.0;
    
    // Disc count
    score += evaluateDiscCount(board, player) * 5.0;
    
    return score;
}

// Positional AI Agent Implementation
PositionalAIAgent::PositionalAIAgent(const std::string& name) : AIAgentBase(name) {
}

std::pair<int, int> PositionalAIAgent::getBestMove(const Board& board, CellState player) {
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    std::pair<int, int> bestMove = validMoves[0];
    double bestScore = std::numeric_limits<double>::lowest();
    
    for (const auto& move : validMoves) {
        double score = evaluatePosition(board, move.first, move.second, player);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    
    return bestMove;
}

double PositionalAIAgent::evaluatePosition(const Board& board, int row, int col, CellState player) const {
    double score = 0.0;
    
    // Check if it's a corner
    if ((row == 0 || row == 7) && (col == 0 || col == 7)) {
        score += CORNER_VALUE;
    }
    // Check if it's an edge (but not corner)
    else if (row == 0 || row == 7 || col == 0 || col == 7) {
        score += EDGE_VALUE;
    }
    // Check if it's in the center area
    else if (row >= 2 && row <= 5 && col >= 2 && col <= 5) {
        score += CENTER_VALUE;
    }
    
    // Penalty for moves adjacent to corners (can give opponent corner access)
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            
            int nr = row + dr;
            int nc = col + dc;
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                if ((nr == 0 || nr == 7) && (nc == 0 || nc == 7)) {
                    if (board.getCell(nr, nc) == CellState::EMPTY) {
                        score += CORNER_ADJACENT_PENALTY;
                    }
                }
            }
        }
    }
    
    // Bonus for moves that flip many discs
    auto flippedDiscs = board.getFlippedDiscs(row, col, player);
    score += flippedDiscs.size() * 2.0;
    
    return score;
}

// Hybrid AI Agent Implementation
HybridAIAgent::HybridAIAgent(const std::string& name, int depth) 
    : AIAgentBase(name), maxDepth(depth) {
}

std::pair<int, int> HybridAIAgent::getBestMove(const Board& board, CellState player) {
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    std::pair<int, int> bestMove = validMoves[0];
    double bestScore = std::numeric_limits<double>::lowest();
    
    for (const auto& move : validMoves) {
        Board tempBoard = board;
        if (tempBoard.makeMove(move.first, move.second, player)) {
            double score = minMax(tempBoard, maxDepth - 1, 
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                player, false);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }
    
    return bestMove;
}

double HybridAIAgent::minMax(Board& board, int depth, double alpha, double beta, 
                            CellState player, bool isMaximizing) {
    if (depth == 0 || board.isGameOver()) {
        return evaluateBoard(board, player);
    }
    
    auto validMoves = getValidMoves(board, isMaximizing ? player : getOpponent(player));
    if (validMoves.empty()) {
        return evaluateBoard(board, player);
    }
    
    if (isMaximizing) {
        double maxScore = std::numeric_limits<double>::lowest();
        for (const auto& move : validMoves) {
            Board tempBoard = board;
            if (tempBoard.makeMove(move.first, move.second, player)) {
                double score = minMax(tempBoard, depth - 1, alpha, beta, player, false);
                maxScore = std::max(maxScore, score);
                alpha = std::max(alpha, score);
                if (beta <= alpha) break;
            }
        }
        return maxScore;
    } else {
        double minScore = std::numeric_limits<double>::max();
        for (const auto& move : validMoves) {
            Board tempBoard = board;
            if (tempBoard.makeMove(move.first, move.second, getOpponent(player))) {
                double score = minMax(tempBoard, depth - 1, alpha, beta, player, true);
                minScore = std::min(minScore, score);
                beta = std::min(beta, score);
                if (beta <= alpha) break;
            }
        }
        return minScore;
    }
}

double HybridAIAgent::evaluateBoard(const Board& board, CellState player) const {
    double score = 0.0;
    
    // Corner control with high weight
    score += evaluateCornerControl(board, player) * CORNER_WEIGHT;
    
    // Edge control
    score += evaluateEdgeControl(board, player) * EDGE_WEIGHT;
    
    // Mobility (ability to make moves)
    score += evaluateMobility(board, player) * MOBILITY_WEIGHT;
    
    // Disc count
    score += evaluateDiscCount(board, player) * DISC_COUNT_WEIGHT;
    
    // Stability
    score += evaluateStability(board, player) * STABILITY_WEIGHT;
    
    return score;
}

// Register all AI agents
REGISTER_AI_AGENT(RandomAIAgent, "random")
REGISTER_AI_AGENT(GreedyAIAgent, "greedy")
REGISTER_AI_AGENT(MinMaxAIAgent, "minmax")
REGISTER_AI_AGENT(PositionalAIAgent, "positional")
REGISTER_AI_AGENT(HybridAIAgent, "hybrid")
