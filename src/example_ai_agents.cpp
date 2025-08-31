#include "example_ai_agents.h"
#include <random>
#include <algorithm>
#include <limits>
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
        Board tempBoard = board;
        if (tempBoard.makeMove(move.first, move.second, player)) {
            int currentScore = tempBoard.getScore(player);
            int originalScore = board.getScore(player);
            int flips = currentScore - originalScore - 1; // -1 because we added our disc
            
            if (flips > maxFlips) {
                maxFlips = flips;
                bestMove = move;
            }
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

double MinMaxAIAgent::evaluateBoard(const Board& board, CellState player) const {
    double score = 0.0;
    
    // Corner control with high weight
    score += evaluateCornerControl(board, player) * 25.0;
    
    // Edge control
    score += evaluateEdgeControl(board, player) * 5.0;
    
    // Mobility (ability to make moves)
    score += evaluateMobility(board, player) * 15.0;
    
    // Disc count
    score += evaluateDiscCount(board, player) * 5.0;
    
    // Stability
    score += evaluateStability(board, player) * 10.0;
    
    return score;
}

// Register all AI agents
REGISTER_AI_AGENT(RandomAIAgent, "random")
REGISTER_AI_AGENT(GreedyAIAgent, "greedy")
REGISTER_AI_AGENT(MinMaxAIAgent, "minmax")
