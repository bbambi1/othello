#pragma once

#include "ai_agent_base.h"
#include <chrono>

// Simple random AI agent - chooses moves randomly from valid options
// Author: Tournament System Example
class RandomAIAgent : public AIAgentBase {
public:
    RandomAIAgent(const std::string& name = "Random", const std::string& author = "Tournament System Example");
    std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                   std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) override;
};
