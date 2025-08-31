#pragma once

#include "board.h"
#include <memory>
#include <chrono>

class AIAgent {
public:
    AIAgent(int maxDepth = 6);
    
    // Get the best move for the current player
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(5000));
    
    // Set the maximum search depth
    void setMaxDepth(int depth);
    
    // Get current search statistics
    int getNodesExplored() const;
    double getAverageTimePerMove() const;
    
private:
    int maxDepth;
    int nodesExplored;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::milliseconds timeLimit;
    
    // MinMax algorithm with alpha-beta pruning
    double minMax(Board& board, int depth, double alpha, double beta, 
                  CellState player, bool isMaximizing, std::chrono::steady_clock::time_point startTime);
    
    // Evaluation function for board state
    double evaluateBoard(const Board& board, CellState player) const;
    
    // Check if time limit has been reached
    bool isTimeUp() const;
    
    // Get opponent color
    static CellState getOpponent(CellState player);
    
    // Heuristic evaluation weights
    static constexpr double CORNER_WEIGHT = 25.0;
    static constexpr double EDGE_WEIGHT = 5.0;
    static constexpr double MOBILITY_WEIGHT = 1.0;
    static constexpr double DISC_COUNT_WEIGHT = 1.0;
};
