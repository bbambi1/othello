#pragma once

#include "ai_agent_base.h"
#include "safe_ai_agent.h"
#include "board.h"
#include <vector>
#include <string>
#include <memory>
#include <fstream>

struct TournamentResult {
    std::string agentName;
    int gamesPlayed;
    int wins;
    int losses;
    int draws;
    double winRate;
    int totalScore;
    double averageScore;
};

struct GameResult {
    std::string blackAgent;
    std::string whiteAgent;
    int blackScore;
    int whiteScore;
    std::string winner;
    int moveCount;
    std::string gameLog;
};

class TournamentManager {
public:
    TournamentManager();
    
    // Add AI agents to the tournament
    void addAgent(std::unique_ptr<AIAgentBase> agent);
    void addAgent(const std::string& type, const std::string& name = "");
    
    // Add agent with safety wrapper
    void addAgentWithSafety(std::unique_ptr<AIAgentBase> agent, const SafetyConfig& config = SafetyConfig());
    
    // Tournament configuration
    void setRoundsPerMatchup(int rounds) { roundsPerMatchup = rounds; }
    void setTimeLimit(std::chrono::milliseconds limit) { timeLimit = limit; }
    void setLogGames(bool log) { logGames = log; }
    void setLogFile(const std::string& filename) { logFilename = filename; }
    
    // Safety configuration
    void setSafetyConfig(const SafetyConfig& config) { safetyConfig = config; }
    const SafetyConfig& getSafetyConfig() const { return safetyConfig; }
    
    // Run tournaments
    void runRoundRobin(int roundsPerMatchup = 1);
    void runSingleElimination();
    void runSwissSystem(int rounds = 5);
    
    // Results and statistics
    std::vector<TournamentResult> getResults() const;
    void printResults() const;
    void saveResults(const std::string& filename) const;
    void saveGameLogs(const std::string& filename) const;
    
    // Individual agent statistics (authoritative source)
    TournamentResult getAgentStats(const std::string& agentName) const;
    
    // Safety statistics
    void printSafetyViolations() const;
    std::vector<std::string> getAgentsWithViolations() const;
    
    // Individual game management
    GameResult playGame(AIAgentBase* blackAgent, AIAgentBase* whiteAgent);
    GameResult playGame(const std::string& blackAgentType, const std::string& whiteAgentType);
    
    // Utility functions
    void resetTournament();
    int getTotalGames() const { return totalGames; }
    double getAverageGameLength() const;
    
private:
    std::vector<std::unique_ptr<AIAgentBase>> agents;
    std::vector<GameResult> gameResults;
    std::vector<TournamentResult> tournamentResults;
    
    int roundsPerMatchup;
    std::chrono::milliseconds timeLimit;
    bool logGames;
    std::string logFilename;
    int totalGames;
    SafetyConfig safetyConfig;
    
    // Helper functions
    void updateTournamentResults();
    void logGame(const GameResult& result);
    std::string formatGameLog(const Board& board, int moveNumber, 
                             const std::string& player, int row, int col) const;
    void printProgressBar(int current, int total) const;
    
    // Tournament algorithms
    void runRoundRobinInternal(int rounds);
    void runSingleEliminationInternal();
    void runSwissSystemInternal(int rounds);
    
    // Swiss system helper functions
    std::vector<std::pair<int, int>> generateSwissPairings(const std::vector<int>& scores) const;
    int calculateSwissScore(int agentIndex) const;
};
