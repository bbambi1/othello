#include "ai_agent.h"
#include <algorithm>
#include <limits>
#include <chrono>

AIAgent::AIAgent(int maxDepth) 
    : maxDepth(maxDepth), nodesExplored(0) {
}

void AIAgent::setMaxDepth(int depth) {
    maxDepth = depth;
}

std::pair<int, int> AIAgent::getBestMove(const Board& board, CellState player, 
                                        std::chrono::milliseconds timeLimit) {
    this->timeLimit = timeLimit;
    startTime = std::chrono::steady_clock::now();
    nodesExplored = 0;
    
    auto validMoves = board.getValidMoves(player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    double bestValue = std::numeric_limits<double>::lowest();
    std::pair<int, int> bestMove = validMoves[0];
    
    // Create a copy of the board for simulation
    Board tempBoard = board;
    
    for (const auto& move : validMoves) {
        if (isTimeUp()) break;
        
        // Make the move
        tempBoard.makeMove(move.first, move.second, player);
        
        // Evaluate the move
        double value = minMax(tempBoard, maxDepth - 1, 
                            std::numeric_limits<double>::lowest(),
                            std::numeric_limits<double>::max(),
                            player, false, startTime);
        
        // Undo the move
        tempBoard = board;
        
        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
        }
        
        if (isTimeUp()) break;
    }
    
    return bestMove;
}

double AIAgent::minMax(Board& board, int depth, double alpha, double beta, 
                      CellState player, bool isMaximizing, std::chrono::steady_clock::time_point startTime) {
    if (isTimeUp()) {
        return 0.0; // Return neutral value if time is up
    }
    
    nodesExplored++;
    
    // Terminal conditions
    if (depth == 0 || board.isGameOver()) {
        return evaluateBoard(board, player);
    }
    
    CellState currentPlayer = isMaximizing ? player : getOpponent(player);
    auto validMoves = board.getValidMoves(currentPlayer);
    
    if (validMoves.empty()) {
        // Pass turn
        if (board.hasValidMoves(getOpponent(currentPlayer))) {
            return minMax(board, depth - 1, alpha, beta, player, !isMaximizing, startTime);
        } else {
            // Game over
            return evaluateBoard(board, player);
        }
    }
    
    if (isMaximizing) {
        double maxValue = std::numeric_limits<double>::lowest();
        
        for (const auto& move : validMoves) {
            if (isTimeUp()) break;
            
            // Make the move
            board.makeMove(move.first, move.second, currentPlayer);
            
            double value = minMax(board, depth - 1, alpha, beta, player, false, startTime);
            
            // Undo the move
            board = board; // This will need to be fixed with proper move history
            
            maxValue = std::max(maxValue, value);
            alpha = std::max(alpha, value);
            
            if (beta <= alpha) {
                break; // Beta cutoff
            }
            
            if (isTimeUp()) break;
        }
        
        return maxValue;
    } else {
        double minValue = std::numeric_limits<double>::max();
        
        for (const auto& move : validMoves) {
            if (isTimeUp()) break;
            
            // Make the move
            board.makeMove(move.first, move.second, currentPlayer);
            
            double value = minMax(board, depth - 1, alpha, beta, player, true, startTime);
            
            // Undo the move
            board = board; // This will need to be fixed with proper move history
            
            minValue = std::min(minValue, value);
            beta = std::min(beta, value);
            
            if (beta <= alpha) {
                break; // Alpha cutoff
            }
            
            if (isTimeUp()) break;
        }
        
        return minValue;
    }
}

double AIAgent::evaluateBoard(const Board& board, CellState player) const {
    if (board.isGameOver()) {
        int playerScore = board.getScore(player);
        int opponentScore = board.getScore(getOpponent(player));
        
        if (playerScore > opponentScore) {
            return 1000.0; // Win
        } else if (playerScore < opponentScore) {
            return -1000.0; // Loss
        } else {
            return 0.0; // Draw
        }
    }
    
    double score = 0.0;
    CellState opponent = getOpponent(player);
    
    // Corner control (most important)
    if (board.getCell(0, 0) == player) score += CORNER_WEIGHT;
    if (board.getCell(0, 7) == player) score += CORNER_WEIGHT;
    if (board.getCell(7, 0) == player) score += CORNER_WEIGHT;
    if (board.getCell(7, 7) == player) score += CORNER_WEIGHT;
    
    if (board.getCell(0, 0) == opponent) score -= CORNER_WEIGHT;
    if (board.getCell(0, 7) == opponent) score -= CORNER_WEIGHT;
    if (board.getCell(7, 0) == opponent) score -= CORNER_WEIGHT;
    if (board.getCell(7, 7) == opponent) score -= CORNER_WEIGHT;
    
    // Edge control
    for (int i = 1; i < 7; ++i) {
        if (board.getCell(0, i) == player) score += EDGE_WEIGHT;
        if (board.getCell(7, i) == player) score += EDGE_WEIGHT;
        if (board.getCell(i, 0) == player) score += EDGE_WEIGHT;
        if (board.getCell(i, 7) == player) score += EDGE_WEIGHT;
        
        if (board.getCell(0, i) == opponent) score -= EDGE_WEIGHT;
        if (board.getCell(7, i) == opponent) score -= EDGE_WEIGHT;
        if (board.getCell(i, 0) == opponent) score -= EDGE_WEIGHT;
        if (board.getCell(i, 7) == opponent) score -= EDGE_WEIGHT;
    }
    
    // Mobility (number of valid moves)
    int playerMobility = board.getValidMoves(player).size();
    int opponentMobility = board.getValidMoves(opponent).size();
    score += (playerMobility - opponentMobility) * MOBILITY_WEIGHT;
    
    // Disc count difference
    int playerDiscs = board.getScore(player);
    int opponentDiscs = board.getScore(opponent);
    score += (playerDiscs - opponentDiscs) * DISC_COUNT_WEIGHT;
    
    return score;
}

bool AIAgent::isTimeUp() const {
    auto elapsed = std::chrono::steady_clock::now() - startTime;
    return elapsed >= timeLimit;
}

CellState AIAgent::getOpponent(CellState player) {
    return (player == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
}

int AIAgent::getNodesExplored() const {
    return nodesExplored;
}

double AIAgent::getAverageTimePerMove() const {
    // This would need to be implemented with move timing history
    return 0.0;
}
