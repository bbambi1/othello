#pragma once

#include "ai_agent_base.h"
#include <chrono>

// Simple random AI agent - chooses moves randomly from valid options
class RandomAIAgent : public AIAgentBase {
public:
    RandomAIAgent(const std::string& name = "Random");
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
};

// Greedy AI agent - always chooses the move that flips the most discs
class GreedyAIAgent : public AIAgentBase {
public:
    GreedyAIAgent(const std::string& name = "Greedy");
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
};

// MinMax AI agent with alpha-beta pruning - the original AI from the project
class MinMaxAIAgent : public AIAgentBase {
public:
    MinMaxAIAgent(const std::string& name = "MinMax", int maxDepth = 6);
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
private:
    int maxDepth;
    double minMax(Board& board, int depth, double alpha, double beta, 
                  CellState player, bool isMaximizing);
    double evaluateBoard(const Board& board, CellState player) const;
};
