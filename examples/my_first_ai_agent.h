#pragma once

#include "ai_agent_base.h"
#include <random>

// This is a simple example AI agent that participants can use as a starting point
// It implements a basic strategy: prefer corners, then edges, then interior positions
class MyFirstAIAgent : public AIAgentBase {
public:
    MyFirstAIAgent(const std::string& name = "MyFirstAgent");
    
    // This is the main method you must implement!
    // It should return the best move as (row, col) for the given board state and player
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
    // Optional: Override these for additional functionality
    void onGameStart() override;
    void onMoveMade(int row, int col, CellState player) override;
    void onGameEnd(CellState winner) override;
    
private:
    std::mt19937 rng;
    
    // Helper methods for evaluating moves
    double evaluateMove(const Board& board, int row, int col, CellState player) const;
    bool isCorner(int row, int col) const;
    bool isEdge(int row, int col) const;
    bool givesOpponentCornerAccess(const Board& board, int row, int col, CellState player) const;
    double evaluateBoardState(const Board& board, CellState player) const;
};
