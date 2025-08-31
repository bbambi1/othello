#include "malicious_agent.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cstdlib>

MaliciousAIAgent::MaliciousAIAgent(const std::string& name)
    : AIAgentBase(name), violationType("resource") {
}

std::pair<int, int> MaliciousAIAgent::getBestMove(const Board& board, CellState player) {
    // Choose which rule to violate based on the violation type
    if (violationType == "timeout") {
        return violateTimeLimit();
    } else if (violationType == "invalid_move") {
        return violateMoveValidation();
    } else if (violationType == "resource") {
        return violateResourceAccess();
    } else if (violationType == "crash") {
        return causeCrash();
    }
    
    // Default: play a valid move
    auto validMoves = getValidMoves(board, player);
    if (!validMoves.empty()) {
        return validMoves[0];
    }
    
    // No valid moves - pass
    return {-1, -1};
}

std::pair<int, int> MaliciousAIAgent::violateTimeLimit() {
    // Sleep for 2 seconds to exceed the 1-second limit
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    // This should never be reached due to timeout
    return {0, 0};
}

std::pair<int, int> MaliciousAIAgent::violateMoveValidation() {
    // Return an invalid move (out of bounds)
    return {10, 10}; // Board is only 8x8, so (10,10) is invalid
}

std::pair<int, int> MaliciousAIAgent::violateResourceAccess() {
    // Try to access external files
    std::ofstream file("malicious_output.txt");
    if (file.is_open()) {
        file << "This agent is trying to access external resources!" << std::endl;
        file.close();
    }
    
    // Try to execute system commands
    system("echo 'Malicious agent detected!'");
    
    // Return a valid move to avoid immediate detection
    return {0, 0};
}

std::pair<int, int> MaliciousAIAgent::causeCrash() {
    // Intentionally cause a crash by throwing an exception
    throw std::runtime_error("Intentionally crashing for testing purposes");
    
    // This should never be reached
    return {0, 0};
}
