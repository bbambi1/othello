#include "minmax_ai_agent.h"
#include <algorithm>
#include <limits>
#include <chrono>

MinMaxAIAgent::MinMaxAIAgent(const std::string& name, const std::string& author, int depth) 
    : AIAgentBase(name, author), maxDepth(depth) {
}

std::pair<int, int> MinMaxAIAgent::getBestMove(const Board& board, CellState player, 
                                               std::chrono::milliseconds timeLimit) {
    auto startTime = std::chrono::steady_clock::now();
    
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1};
    }
    
    std::pair<int, int> bestMove = validMoves[0];
    double bestScore = std::numeric_limits<double>::lowest();
    
    for (const auto& move : validMoves) {
        if (isTimeUp(startTime, timeLimit)) {
            break;
        }
        
        Board tempBoard = board;
        if (tempBoard.makeMove(move.first, move.second, player)) {
            double score = minMax(tempBoard, maxDepth - 1, 
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                player, false, startTime, timeLimit);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }
    
    return bestMove;
}

double MinMaxAIAgent::minMax(Board& board, int depth, double alpha, double beta, 
                            CellState player, bool isMaximizing,
                            std::chrono::steady_clock::time_point startTime,
                            std::chrono::milliseconds timeLimit) {
    if (isTimeUp(startTime, timeLimit)) {
        return 0.0;
    }
    
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
            if (isTimeUp(startTime, timeLimit)) {
                break;
            }
            
            Board tempBoard = board;
            if (tempBoard.makeMove(move.first, move.second, player)) {
                double score = minMax(tempBoard, depth - 1, alpha, beta, player, false, startTime, timeLimit);
                maxScore = std::max(maxScore, score);
                alpha = std::max(alpha, score);
                if (beta <= alpha) break;
            }
        }
        return maxScore;
    } else {
        double minScore = std::numeric_limits<double>::max();
        for (const auto& move : validMoves) {
            if (isTimeUp(startTime, timeLimit)) {
                break;
            }
            
            Board tempBoard = board;
            if (tempBoard.makeMove(move.first, move.second, getOpponent(player))) {
                double score = minMax(tempBoard, depth - 1, alpha, beta, player, true, startTime, timeLimit);
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
    
    score += evaluateCornerControl(board, player);
    score += evaluateEdgeControl(board, player);
    score += evaluateMobility(board, player);
    score += evaluateDiscCount(board, player);
    score += evaluateStability(board, player);
    
    return score;
}

double MinMaxAIAgent::evaluateCornerControl(const Board& board, CellState player) const {
    static const std::vector<std::pair<int, int>> corners = {
        {0, 0}, {0, 7}, {7, 0}, {7, 7}
    };
    
    double score = 0.0;
    for (const auto& corner : corners) {
        CellState cellState = board.getCell(corner.first, corner.second);
        if (cellState == player) {
            score += 25.0;
        } else if (cellState == getOpponent(player)) {
            score -= 25.0;
        }
    }
    return score;
}

double MinMaxAIAgent::evaluateEdgeControl(const Board& board, CellState player) const {
    double score = 0.0;
    
    for (int i = 1; i < 7; ++i) {
        // Top edge
        if (board.getCell(0, i) == player) score += 5.0;
        else if (board.getCell(0, i) == getOpponent(player)) score -= 5.0;
        
        // Bottom edge
        if (board.getCell(7, i) == player) score += 5.0;
        else if (board.getCell(7, i) == getOpponent(player)) score -= 5.0;
        
        // Left edge
        if (board.getCell(i, 0) == player) score += 5.0;
        else if (board.getCell(i, 0) == getOpponent(player)) score -= 5.0;
        
        // Right edge
        if (board.getCell(i, 7) == player) score += 5.0;
        else if (board.getCell(i, 7) == getOpponent(player)) score -= 5.0;
    }
    
    return score;
}

double MinMaxAIAgent::evaluateMobility(const Board& board, CellState player) const {
    int playerMoves = board.getValidMoves(player).size();
    int opponentMoves = board.getValidMoves(getOpponent(player)).size();
    
    if (playerMoves + opponentMoves == 0) return 0.0;
    
    return static_cast<double>(playerMoves - opponentMoves) / (playerMoves + opponentMoves) * 15.0;
}

double MinMaxAIAgent::evaluateDiscCount(const Board& board, CellState player) const {
    int playerDiscs = board.getScore(player);
    int opponentDiscs = board.getScore(getOpponent(player));
    int totalDiscs = board.getTotalDiscs();
    
    if (totalDiscs == 0) return 0.0;
    
    return static_cast<double>(playerDiscs - opponentDiscs) / totalDiscs * 5.0;
}

double MinMaxAIAgent::evaluateStability(const Board& board, CellState player) const {
    double score = 0.0;
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board.getCell(row, col) == player) {
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
                    score -= 2.0;
                } else {
                    score += 1.0;
                }
            }
        }
    }
    
    return score * 10.0;
}

REGISTER_AI_AGENT(MinMaxAIAgent, "minmax")
