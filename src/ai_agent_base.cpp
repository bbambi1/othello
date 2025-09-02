#include "ai_agent_base.h"
#include <chrono>
#include <algorithm>
#include <map>
#include <iostream>

// Global registry for AI agents
static std::map<std::string, AIAgentCreator> aiAgentRegistry;

AIAgentBase::AIAgentBase(const std::string& agentName, const std::string& agentAuthor) 
    : name(agentName), author(agentAuthor) {
}

void AIAgentBase::onGameStart() {
    // Default implementation does nothing
}

void AIAgentBase::onMoveMade(int row, int col, CellState player) {
    // Default implementation does nothing
    (void)row;   // Suppress unused parameter warning
    (void)col;   // Suppress unused parameter warning
    (void)player; // Suppress unused parameter warning
}

void AIAgentBase::onGameEnd(CellState winner) {
    // Default implementation does nothing
    (void)winner; // Suppress unused parameter warning
}

std::vector<std::pair<int, int>> AIAgentBase::getValidMoves(const Board& board, CellState player) const {
    return board.getValidMoves(player);
}

bool AIAgentBase::isValidMove(const Board& board, int row, int col, CellState player) const {
    return board.isValidMove(row, col, player);
}

CellState AIAgentBase::getOpponent(CellState player) const {
    return (player == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
}



bool AIAgentBase::isTimeUp(std::chrono::steady_clock::time_point startTime, 
                           std::chrono::milliseconds timeLimit) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return elapsed >= timeLimit;
}

// Factory implementation
std::unique_ptr<AIAgentBase> createAIAgent(const std::string& type, const std::string& name, const std::string& author) {
    auto it = aiAgentRegistry.find(type);
    if (it != aiAgentRegistry.end()) {
        return it->second(name, author);
    }
    
    std::cerr << "Unknown AI agent type: " << type << std::endl;
    std::cerr << "Available types:" << std::endl;
    for (const auto& pair : aiAgentRegistry) {
        std::cerr << "  " << pair.first << std::endl;
    }
    
    return nullptr;
}

void registerAIAgent(const std::string& type, AIAgentCreator creator) {
    aiAgentRegistry[type] = creator;
}
