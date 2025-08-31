#include "safe_ai_agent.h"
#include <iostream>
#include <stdexcept>
#include <system_error>

SafeAIAgent::SafeAIAgent(std::unique_ptr<AIAgentBase> agent, const SafetyConfig& config)
    : AIAgentBase(agent->getName()), wrappedAgent(std::move(agent)), config(config) {
    // Initialize the base class with the wrapped agent's original name
}

std::pair<int, int> SafeAIAgent::getBestMove(const Board& board, CellState player) {
    // Start timing
    moveStartTime = std::chrono::steady_clock::now();
    
    try {
        // Monitor resource access if enabled
        if (config.enableResourceMonitoring) {
            monitorResourceAccess();
        }
        
        // Get move from wrapped agent
        auto move = wrappedAgent->getBestMove(board, player);
        
        // Check time limit if enabled
        if (config.enableTimeLimit && !checkTimeLimit(moveStartTime)) {
            timeLimitViolations++;
            recordViolation("Exceeded time limit");
            throw TimeLimitExceededException();
        }
        
        // Validate move if enabled
        if (config.enableMoveValidation && !validateMove(board, move.first, move.second, player)) {
            invalidMoveViolations++;
            recordViolation("Played invalid move");
            throw InvalidMoveException(move.first, move.second);
        }
        
        return move;
        
    } catch (const std::exception& e) {
        // Re-throw our specific exceptions
        if (dynamic_cast<const TimeLimitExceededException*>(&e) ||
            dynamic_cast<const InvalidMoveException*>(&e) ||
            dynamic_cast<const AgentCrashException*>(&e) ||
            dynamic_cast<const ResourceAccessException*>(&e)) {
            throw;
        }
        
        // Any other exception is treated as a crash
        crashViolations++;
        recordViolation("Crashed with exception: " + std::string(e.what()));
        throw AgentCrashException();
    } catch (...) {
        // Unknown exception type
        crashViolations++;
        recordViolation("Crashed with unknown exception");
        throw AgentCrashException();
    }
}

void SafeAIAgent::onGameStart() {
    // Reset safety statistics at the start of each game
    resetSafetyStats();
    
    if (wrappedAgent) {
        try {
            wrappedAgent->onGameStart();
        } catch (...) {
            crashViolations++;
            recordViolation("Crashed in onGameStart");
        }
    }
}

void SafeAIAgent::onMoveMade(int row, int col, CellState player) {
    if (wrappedAgent) {
        try {
            wrappedAgent->onMoveMade(row, col, player);
        } catch (...) {
            crashViolations++;
            recordViolation("Crashed in onMoveMade");
        }
    }
}

void SafeAIAgent::onGameEnd(CellState winner) {
    if (wrappedAgent) {
        try {
            wrappedAgent->onGameEnd(winner);
        } catch (...) {
            crashViolations++;
            recordViolation("Crashed in onGameEnd");
        }
    }
}

const std::string& SafeAIAgent::getWrappedAgentName() const {
    if (wrappedAgent) {
        return wrappedAgent->getName();
    }
    static std::string emptyName;
    return emptyName;
}

void SafeAIAgent::resetSafetyStats() {
    timeLimitViolations = 0;
    invalidMoveViolations = 0;
    crashViolations = 0;
    resourceViolations = 0;
}

void SafeAIAgent::recordViolation(const std::string& reason) {
    std::cerr << "Agent " << getName() << " violation: " << reason << std::endl;
}

std::string SafeAIAgent::getViolationSummary() const {
    std::string summary;
    if (timeLimitViolations > 0) {
        summary += "Time limit violations: " + std::to_string(timeLimitViolations) + "; ";
    }
    if (invalidMoveViolations > 0) {
        summary += "Invalid moves: " + std::to_string(invalidMoveViolations) + "; ";
    }
    if (crashViolations > 0) {
        summary += "Crashes: " + std::to_string(crashViolations) + "; ";
    }
    if (resourceViolations > 0) {
        summary += "Resource violations: " + std::to_string(resourceViolations) + "; ";
    }
    if (summary.empty()) {
        summary = "No violations";
    }
    return summary;
}

bool SafeAIAgent::shouldForfeitGame() const {
    // Agent should forfeit if it has any violations
    return hasViolations();
}

bool SafeAIAgent::checkTimeLimit(std::chrono::steady_clock::time_point startTime) {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    return elapsed <= config.timeLimit;
}

bool SafeAIAgent::validateMove(const Board& board, int row, int col, CellState player) {
    return board.isValidMove(row, col, player);
}

void SafeAIAgent::monitorResourceAccess() {
    // Basic resource monitoring - can be enhanced with more sophisticated checks
    // For now, we'll just check if the agent is trying to access files or network
    
    // This is a placeholder for more advanced resource monitoring
    // In a real implementation, you might:
    // - Monitor file system access
    // - Check network connections
    // - Monitor memory usage patterns
    // - Check for suspicious system calls
    
    // For now, we'll assume the agent is well-behaved
    // This can be enhanced later with platform-specific monitoring
}
