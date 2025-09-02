#pragma once

#include "ai_agent_base.h"
#include <chrono>

// MinMax AI agent with alpha-beta pruning - the original AI from the project
// Author: Tournament System Example
class MinMaxAIAgent : public AIAgentBase {
public:
    MinMaxAIAgent(const std::string& name = "MinMax", const std::string& author = "Tournament System Example", int maxDepth = 6);
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
    
private:
    int maxDepth;
    double minMax(Board& board, int depth, double alpha, double beta, 
                  CellState player, bool isMaximizing, 
                  std::chrono::steady_clock::time_point startTime,
                  std::chrono::milliseconds timeLimit);
    double evaluateBoard(const Board& board, CellState player) const;
    
    // Private evaluation methods for MinMax strategy
    double evaluateCornerControl(const Board& board, CellState player) const;
    double evaluateEdgeControl(const Board& board, CellState player) const;
    double evaluateMobility(const Board& board, CellState player) const;
    double evaluateDiscCount(const Board& board, CellState player) const;
    double evaluateStability(const Board& board, CellState player) const;
};
