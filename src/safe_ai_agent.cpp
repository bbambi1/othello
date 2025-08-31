#include "safe_ai_agent.h"
#include <iostream>
#include <stdexcept>
#include <system_error>

SafeAIAgent::SafeAIAgent(std::unique_ptr<AIAgentBase> agent, const SafetyConfig& config)
    : AIAgentBase(agent->getName()), wrappedAgent(std::move(agent)), config(config) {
    // Initialize the base class with the wrapped agent's original name
}

std::pair<int, int> SafeAIAgent::getBestMove(const Board& board, CellState player) {
    // Check if already disqualified
    if (disqualified) {
        throw AgentCrashException();
    }
    
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
            disqualifyAgent("Exceeded time limit");
            throw TimeLimitExceededException();
        }
        
        // Validate move if enabled
        if (config.enableMoveValidation && !validateMove(board, move.first, move.second, player)) {
            invalidMoveViolations++;
            disqualifyAgent("Played invalid move");
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
        disqualifyAgent("Crashed with exception: " + std::string(e.what()));
        throw AgentCrashException();
    } catch (...) {
        // Unknown exception type
        crashViolations++;
        disqualifyAgent("Crashed with unknown exception");
        throw AgentCrashException();
    }
}

void SafeAIAgent::onGameStart() {
    if (!disqualified && wrappedAgent) {
        try {
            wrappedAgent->onGameStart();
        } catch (...) {
            crashViolations++;
            disqualifyAgent("Crashed in onGameStart");
        }
    }
}

void SafeAIAgent::onMoveMade(int row, int col, CellState player) {
    if (!disqualified && wrappedAgent) {
        try {
            wrappedAgent->onMoveMade(row, col, player);
        } catch (...) {
            crashViolations++;
            disqualifyAgent("Crashed in onMoveMade");
        }
    }
}

void SafeAIAgent::onGameEnd(CellState winner) {
    if (!disqualified && wrappedAgent) {
        try {
            wrappedAgent->onGameEnd(winner);
        } catch (...) {
            crashViolations++;
            disqualifyAgent("Crashed in onGameEnd");
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
    disqualified = false;
    disqualificationReason.clear();
}

void SafeAIAgent::disqualifyAgent(const std::string& reason) {
    disqualified = true;
    disqualificationReason = reason;
    std::cerr << "Agent " << getName() << " disqualified: " << reason << std::endl;
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
