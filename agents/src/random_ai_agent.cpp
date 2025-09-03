#include "random_ai_agent.h"
#include <random>
#include <chrono>

RandomAIAgent::RandomAIAgent(const std::string& name, const std::string& author) 
    : AIAgentBase(name, author) {
}

std::pair<int, int> RandomAIAgent::getBestMove(const Board& board, CellState player, 
                                               std::chrono::milliseconds timeLimit) {
    auto startTime = std::chrono::steady_clock::now();
    
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1};
    }
    
    if (isTimeUp(startTime, timeLimit)) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, validMoves.size() - 1);
        return validMoves[dis(gen)];
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, validMoves.size() - 1);
    
    return validMoves[dis(gen)];
}

REGISTER_AI_AGENT(RandomAIAgent, "random")
