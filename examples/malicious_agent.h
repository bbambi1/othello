#pragma once

#include "ai_agent_base.h"
#include <chrono>
#include <thread>

// This agent intentionally violates competition rules to test the safety system
// WARNING: This is for testing purposes only - do not use in real competitions!
class MaliciousAIAgent : public AIAgentBase {
public:
    MaliciousAIAgent(const std::string& name);
    ~MaliciousAIAgent() override = default;
    
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
    // Set which rule to violate
    void setViolationType(const std::string& type) { violationType = type; }
    
private:
    std::string violationType; // "timeout", "invalid_move", "crash", "resource"
    
    // Helper methods for different violations
    std::pair<int, int> violateTimeLimit();
    std::pair<int, int> violateMoveValidation();
    std::pair<int, int> violateResourceAccess();
    std::pair<int, int> causeCrash();
};
