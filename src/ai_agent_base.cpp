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

double AIAgentBase::evaluateCornerControl(const Board& board, CellState player) const {
    static const std::vector<std::pair<int, int>> corners = {
        {0, 0}, {0, 7}, {7, 0}, {7, 7}
    };
    
    double score = 0.0;
    for (const auto& corner : corners) {
        CellState cellState = board.getCell(corner.first, corner.second);
        if (cellState == player) {
            score += 25.0; // Corner is very valuable
        } else if (cellState == getOpponent(player)) {
            score -= 25.0;
        }
    }
    return score;
}

double AIAgentBase::evaluateEdgeControl(const Board& board, CellState player) const {
    double score = 0.0;
    
    // Evaluate edges (excluding corners)
    for (int i = 1; i < 7; ++i) {
        // Top edge
        if (board.getCell(0, i) == player) score += 5.0;
        else if (board.getCell(0, i) == getOpponent(player)) score -= 5.0;
        
        // Bottom edge
        if (board.getCell(7, i) == player) score += 5.0;
        else if (board.getCell(7, i) == getOpponent(player)) score -= 5.0;
        
        // Left edge
        if (board.getCell(i, 0) == player) score += 5.0;
        else if (board.getCell(i, 0) == getOpponent(player)) score -= 5.0;
        
        // Right edge
        if (board.getCell(i, 7) == player) score += 5.0;
        else if (board.getCell(i, 7) == getOpponent(player)) score -= 5.0;
    }
    
    return score;
}

double AIAgentBase::evaluateMobility(const Board& board, CellState player) const {
    int playerMoves = board.getValidMoves(player).size();
    int opponentMoves = board.getValidMoves(getOpponent(player)).size();
    
    if (playerMoves + opponentMoves == 0) return 0.0;
    
    return static_cast<double>(playerMoves - opponentMoves) / (playerMoves + opponentMoves);
}

double AIAgentBase::evaluateDiscCount(const Board& board, CellState player) const {
    int playerDiscs = board.getScore(player);
    int opponentDiscs = board.getScore(getOpponent(player));
    int totalDiscs = board.getTotalDiscs();
    
    if (totalDiscs == 0) return 0.0;
    
    return static_cast<double>(playerDiscs - opponentDiscs) / totalDiscs;
}

double AIAgentBase::evaluateStability(const Board& board, CellState player) const {
    // Simple stability evaluation based on corner adjacency
    double score = 0.0;
    
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board.getCell(row, col) == player) {
                // Check if disc is adjacent to corners (less stable)
                bool adjacentToCorner = false;
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        int nr = row + dr;
                        int nc = col + dc;
                        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                            if ((nr == 0 || nr == 7) && (nc == 0 || nc == 7)) {
                                adjacentToCorner = true;
                                break;
                            }
                        }
                    }
                    if (adjacentToCorner) break;
                }
                
                if (adjacentToCorner) {
                    score -= 2.0; // Less stable
                } else {
                    score += 1.0; // More stable
                }
            }
        }
    }
    
    return score;
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
