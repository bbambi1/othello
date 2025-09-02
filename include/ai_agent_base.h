#pragma once

#include "board.h"
#include <string>
#include <memory>
#include <chrono>

// Base class for AI agents in the competition
class AIAgentBase {
public:
    AIAgentBase(const std::string& name, const std::string& author = "");
    virtual ~AIAgentBase() = default;
    
    // Main method that participants must implement
    // Returns the best move as (row, col) for the given board state and player
    // timeLimit: maximum time allowed for the move (default: 1 second)
    virtual std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                           std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) = 0;
    
    // Optional methods that can be overridden for additional functionality
    virtual void onGameStart();
    virtual void onMoveMade(int row, int col, CellState player);
    virtual void onGameEnd(CellState winner);
    
    // Utility methods
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    const std::string& getAuthor() const { return author; }
    void setAuthor(const std::string& newAuthor) { author = newAuthor; }
    
protected:
    std::string name;
    std::string author;
    
    // Helper methods that derived classes can use
    std::vector<std::pair<int, int>> getValidMoves(const Board& board, CellState player) const;
    bool isValidMove(const Board& board, int row, int col, CellState player) const;
    CellState getOpponent(CellState player) const;
    

    
    // Time management
    bool isTimeUp(std::chrono::steady_clock::time_point startTime, 
                  std::chrono::milliseconds timeLimit = std::chrono::milliseconds(1000)) const;
};

// Function pointer type for AI agent creation (defined after AIAgentBase)
using AIAgentCreator = std::unique_ptr<AIAgentBase>(*)(const std::string&, const std::string&);

// Factory function for creating AI agents
std::unique_ptr<AIAgentBase> createAIAgent(const std::string& type, const std::string& name = "", const std::string& author = "");

// Registration macro for AI agents
#define REGISTER_AI_AGENT(ClassName, TypeName) \
    namespace { \
        class ClassName##Factory { \
        public: \
            ClassName##Factory() { \
                registerAIAgent(TypeName, [](const std::string& name, const std::string& author) -> std::unique_ptr<AIAgentBase> { \
                    return std::make_unique<ClassName>(name, author); \
                }); \
            } \
        }; \
        static ClassName##Factory ClassName##FactoryInstance; \
    }

// Registration function (implemented in ai_agent_base.cpp)
void registerAIAgent(const std::string& type, AIAgentCreator creator);
