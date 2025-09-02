#pragma once

#include "ai_agent_base.h"
#include <chrono>

// Greedy AI agent - always chooses the move that flips the most discs
// Author: Tournament System Example
class GreedyAIAgent : public AIAgentBase {
public:
    GreedyAIAgent(const std::string& name = "Greedy", const std::string& author = "Tournament System Example");
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
};
