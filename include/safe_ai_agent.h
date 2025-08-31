#pragma once

#include "ai_agent_base.h"
#include "board.h"
#include <chrono>
#include <memory>
#include <string>
#include <exception>

// Exception types for different rule violations
class TimeLimitExceededException : public std::exception {
public:
    const char* what() const noexcept override {
        return "AI agent exceeded time limit";
    }
};

class InvalidMoveException : public std::exception {
public:
    InvalidMoveException(int row, int col) : row_(row), col_(col) {}
    
    const char* what() const noexcept override {
        return "AI agent played invalid move";
    }
    
    int getRow() const { return row_; }
    int getCol() const { return col_; }
    
private:
    int row_, col_;
};

class AgentCrashException : public std::exception {
public:
    const char* what() const noexcept override {
        return "AI agent crashed or behaved unexpectedly";
    }
};

class ResourceAccessException : public std::exception {
public:
    const char* what() const noexcept override {
        return "AI agent attempted to access external resources";
    }
};

// Configuration for safety constraints
struct SafetyConfig {
    std::chrono::milliseconds timeLimit{1000}; // Default: 1 second
    bool enableCrashDetection{true};
    bool enableResourceMonitoring{true};
    bool enableMoveValidation{true};
    bool enableTimeLimit{true};
    
    // Constructor with custom time limit
    SafetyConfig(std::chrono::milliseconds limit = std::chrono::milliseconds(1000))
        : timeLimit(limit) {}
};

// Safe wrapper for AI agents that enforces competition rules
class SafeAIAgent : public AIAgentBase {
public:
    SafeAIAgent(std::unique_ptr<AIAgentBase> agent, const SafetyConfig& config = SafetyConfig());
    ~SafeAIAgent() override = default;
    
    // Override the main method with safety checks
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
    // Forward other methods to the wrapped agent
    void onGameStart() override;
    void onMoveMade(int row, int col, CellState player) override;
    void onGameEnd(CellState winner) override;
    
    // Get the wrapped agent's name
    const std::string& getWrappedAgentName() const;
    
    // Safety statistics
    int getTimeLimitViolations() const { return timeLimitViolations; }
    int getInvalidMoveViolations() const { return invalidMoveViolations; }
    int getCrashViolations() const { return crashViolations; }
    int getResourceViolations() const { return resourceViolations; }
    
    // Reset safety statistics
    void resetSafetyStats();
    
    // Check if agent has violations in current game
    bool hasViolations() const { 
        return timeLimitViolations > 0 || invalidMoveViolations > 0 || 
               crashViolations > 0 || resourceViolations > 0; 
    }
    
    // Get violation summary for current game
    std::string getViolationSummary() const;
    
    // Check if agent should forfeit current game
    bool shouldForfeitGame() const;

private:
    std::unique_ptr<AIAgentBase> wrappedAgent;
    SafetyConfig config;
    
    // Safety statistics
    int timeLimitViolations{0};
    int invalidMoveViolations{0};
    int crashViolations{0};
    int resourceViolations{0};
    
    // Helper methods
    void recordViolation(const std::string& reason);
    bool checkTimeLimit(std::chrono::steady_clock::time_point startTime);
    bool validateMove(const Board& board, int row, int col, CellState player);
    void monitorResourceAccess();
    
    // Time tracking
    std::chrono::steady_clock::time_point moveStartTime;
};
