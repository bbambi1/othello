#pragma once

#include "board.h"
#include <string>
#include <memory>
#include <chrono>

// Base class for AI agents in the competition
class AIAgentBase {
public:
    AIAgentBase(const std::string& name);
    virtual ~AIAgentBase() = default;
    
    // Main method that participants must implement
    // Returns the best move as (row, col) for the given board state and player
    virtual std::pair<int, int> getBestMove(const Board& board, CellState player) = 0;
    
    // Optional methods that can be overridden for additional functionality
    virtual void onGameStart();
    virtual void onMoveMade(int row, int col, CellState player);
    virtual void onGameEnd(CellState winner);
    
    // Utility methods
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    
    // Statistics tracking
    void resetStats();
    int getGamesPlayed() const { return gamesPlayed; }
    int getGamesWon() const { return gamesWon; }
    double getWinRate() const;
    
protected:
    std::string name;
    int gamesPlayed;
    int gamesWon;
    
    // Helper methods that derived classes can use
    std::vector<std::pair<int, int>> getValidMoves(const Board& board, CellState player) const;
    bool isValidMove(const Board& board, int row, int col, CellState player) const;
    CellState getOpponent(CellState player) const;
    
    // Common evaluation functions
    double evaluateCornerControl(const Board& board, CellState player) const;
    double evaluateEdgeControl(const Board& board, CellState player) const;
    double evaluateMobility(const Board& board, CellState player) const;
    double evaluateDiscCount(const Board& board, CellState player) const;
    double evaluateStability(const Board& board, CellState player) const;
    
    // Time management
    bool isTimeUp(std::chrono::steady_clock::time_point startTime, 
                  std::chrono::milliseconds timeLimit) const;
};

// Function pointer type for AI agent creation (defined after AIAgentBase)
using AIAgentCreator = std::unique_ptr<AIAgentBase>(*)(const std::string&);

// Factory function for creating AI agents
std::unique_ptr<AIAgentBase> createAIAgent(const std::string& type, const std::string& name = "");

// Registration macro for AI agents
#define REGISTER_AI_AGENT(ClassName, TypeName) \
    namespace { \
        class ClassName##Factory { \
        public: \
            ClassName##Factory() { \
                registerAIAgent(TypeName, [](const std::string& name) -> std::unique_ptr<AIAgentBase> { \
                    return std::make_unique<ClassName>(name); \
                }); \
            } \
        }; \
        static ClassName##Factory ClassName##FactoryInstance; \
    }

// Registration function (implemented in ai_agent_base.cpp)
void registerAIAgent(const std::string& type, AIAgentCreator creator);
