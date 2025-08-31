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

// Positional AI agent - focuses on controlling strategic positions
class PositionalAIAgent : public AIAgentBase {
public:
    PositionalAIAgent(const std::string& name = "Positional");
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
private:
    double evaluatePosition(const Board& board, int row, int col, CellState player) const;
    static constexpr double CORNER_VALUE = 100.0;
    static constexpr double EDGE_VALUE = 20.0;
    static constexpr double CENTER_VALUE = 5.0;
    static constexpr double CORNER_ADJACENT_PENALTY = -10.0;
};

// Hybrid AI agent - combines multiple strategies
class HybridAIAgent : public AIAgentBase {
public:
    HybridAIAgent(const std::string& name = "Hybrid", int maxDepth = 4);
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
private:
    int maxDepth;
    double minMax(Board& board, int depth, double alpha, double beta, 
                  CellState player, bool isMaximizing);
    double evaluateBoard(const Board& board, CellState player) const;
    
    // Strategy weights
    static constexpr double CORNER_WEIGHT = 30.0;
    static constexpr double EDGE_WEIGHT = 8.0;
    static constexpr double MOBILITY_WEIGHT = 15.0;
    static constexpr double DISC_COUNT_WEIGHT = 5.0;
    static constexpr double STABILITY_WEIGHT = 10.0;
};
