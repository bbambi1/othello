#pragma once

#include "ai_agent_base.h"
#include <chrono>

// Simple random AI agent - chooses moves randomly from valid options
class RandomAIAgent : public AIAgentBase {
public:
    RandomAIAgent(const std::string& name = "Random");
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
};

// Greedy AI agent - always chooses the move that flips the most discs
class GreedyAIAgent : public AIAgentBase {
public:
    GreedyAIAgent(const std::string& name = "Greedy");
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
};

// MinMax AI agent with alpha-beta pruning - the original AI from the project
class MinMaxAIAgent : public AIAgentBase {
public:
    MinMaxAIAgent(const std::string& name = "MinMax", int maxDepth = 6);
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
    
private:
    int maxDepth;
    double minMax(Board& board, int depth, double alpha, double beta, 
                  CellState player, bool isMaximizing, 
                  std::chrono::steady_clock::time_point startTime,
                  std::chrono::milliseconds timeLimit);
    double evaluateBoard(const Board& board, CellState player) const;
};
