#include "simple_tournament.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

SimpleTournament::SimpleTournament() {
    // Initialize with default config
}

void SimpleTournament::addAgent(std::unique_ptr<AIAgentBase> agent) {
    if (agent) {
        agents_.push_back(std::move(agent));
    }
}

void SimpleTournament::addAgent(const std::string& type, const std::string& name) {
    auto agent = createAIAgent(type, name.empty() ? type : name);
    if (agent) {
        agents_.push_back(std::move(agent));
    }
}

void SimpleTournament::clearAgents() {
    agents_.clear();
    gameResults_.clear();
    agentStats_.clear();
}

void SimpleTournament::runRoundRobin() {
    if (agents_.size() < 2) {
        std::cout << "Error: Need at least 2 agents for a tournament" << std::endl;
        return;
    }
    
    // Calculate total games
    int totalMatchups = (agents_.size() * (agents_.size() - 1)) / 2;
    int totalGames = totalMatchups * config_.roundsPerMatchup * 2; // Each round always plays both sides for fairness
    
    std::cout << "Starting Round Robin Tournament" << std::endl;
    std::cout << "Agents: " << agents_.size() << std::endl;
    std::cout << "Rounds per matchup: " << config_.roundsPerMatchup << " (each round plays both sides for fairness)" << std::endl;
    std::cout << "Time limit per move: " << config_.timeLimit.count() << "ms" << std::endl;
    std::cout << "Total games: " << totalGames << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    int currentGame = 0;
    
    // Initialize agent stats
    for (const auto& agent : agents_) {
        agentStats_[agent->getName()] = AgentStats();
        agentStats_[agent->getName()].agentName = agent->getName();
    }
    
    // Play all matchups
    for (size_t i = 0; i < agents_.size(); ++i) {
        for (size_t j = i + 1; j < agents_.size(); ++j) {
            std::string matchup = agents_[i]->getName() + " vs " + agents_[j]->getName();
            std::cout << "\nMatchup: " << matchup << std::endl;
            
            for (int round = 0; round < config_.roundsPerMatchup; ++round) {
                currentGame++;
                
                // Update progress
                if (progressCallback_) {
                    progressCallback_(currentGame, totalGames, matchup);
                }
                printProgress(currentGame, totalGames, matchup);
                
                // Play game with agent i as black
                auto result1 = playSingleGame(agents_[i].get(), agents_[j].get());
                gameResults_.push_back(result1);
                updateStats(result1);
                
                // Always play the reverse game for fairness (both sides)
                currentGame++;
                
                if (progressCallback_) {
                    progressCallback_(currentGame, totalGames, matchup + " (reversed)");
                }
                printProgress(currentGame, totalGames, matchup + " (reversed)");
                
                auto result2 = playSingleGame(agents_[j].get(), agents_[i].get());
                gameResults_.push_back(result2);
                updateStats(result2);
            }
        }
    }
    
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Tournament Complete!" << std::endl;
    printResults();
}

GameResult SimpleTournament::playSingleGame(AIAgentBase* blackAgent, AIAgentBase* whiteAgent) {
    GameResult result;
    result.blackAgent = blackAgent->getName();
    result.whiteAgent = whiteAgent->getName();
    
    Board board;
    CellState currentPlayer = CellState::BLACK;
    int moveCount = 0;
    
    auto gameStart = std::chrono::steady_clock::now();
    
    // Notify agents of game start
    try {
        blackAgent->onGameStart();
        whiteAgent->onGameStart();
    } catch (...) {
        // If agents crash during game start, we'll handle it during moves
    }
    
    // Game loop
    while (!board.isGameOver()) {
        AIAgentBase* currentAgent = (currentPlayer == CellState::BLACK) ? blackAgent : whiteAgent;
        std::string agentName = currentAgent->getName();
        
        // Check if current player has valid moves
        if (!board.hasValidMoves(currentPlayer)) {
            // Player must pass
            currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
            continue;
        }
        
        // Get move from agent with safety checks
        bool timedOut = false, crashed = false;
        auto move = getSafeMove(currentAgent, board, currentPlayer, agentName, timedOut, crashed);
        
        // Handle failures
        if (timedOut || crashed) {
            if (currentPlayer == CellState::BLACK) {
                result.blackTimedOut = timedOut;
                result.blackCrashed = crashed;
                result.winner = "WHITE";
                result.failureReason = agentName + " " + (timedOut ? "timed out" : "crashed");
            } else {
                result.whiteTimedOut = timedOut;
                result.whiteCrashed = crashed;
                result.winner = "BLACK";
                result.failureReason = agentName + " " + (timedOut ? "timed out" : "crashed");
            }
            break;
        }
        
        // Make the move
        if (!board.makeMove(move.first, move.second, currentPlayer)) {
            // Invalid move - agent loses
            if (currentPlayer == CellState::BLACK) {
                result.winner = "WHITE";
                result.failureReason = agentName + " played invalid move";
            } else {
                result.winner = "BLACK";
                result.failureReason = agentName + " played invalid move";
            }
            break;
        }
        
        // Notify agents of the move
        try {
            blackAgent->onMoveMade(move.first, move.second, currentPlayer);
            whiteAgent->onMoveMade(move.first, move.second, currentPlayer);
        } catch (...) {
            // If agents crash during move notification, continue the game
        }
        
        moveCount++;
        currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
    }
    
    // Determine final result if no failure occurred
    if (result.winner.empty()) {
        result.blackScore = board.getScore(CellState::BLACK);
        result.whiteScore = board.getScore(CellState::WHITE);
        
        if (result.blackScore > result.whiteScore) {
            result.winner = "BLACK";
        } else if (result.whiteScore > result.blackScore) {
            result.winner = "WHITE";
        } else {
            result.winner = "DRAW";
        }
    } else {
        // Set scores for failed games
        if (result.winner == "BLACK") {
            result.blackScore = 64;  // Maximum possible score
            result.whiteScore = 0;
        } else if (result.winner == "WHITE") {
            result.blackScore = 0;
            result.whiteScore = 64;
        }
    }
    
    result.moveCount = moveCount;
    auto gameEnd = std::chrono::steady_clock::now();
    result.gameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(gameEnd - gameStart);
    
    // Notify agents of game end
    try {
        CellState winner = (result.winner == "BLACK") ? CellState::BLACK : 
                          (result.winner == "WHITE") ? CellState::WHITE : CellState::EMPTY;
        blackAgent->onGameEnd(winner);
        whiteAgent->onGameEnd(winner);
    } catch (...) {
        // If agents crash during game end notification, ignore it
    }
    
    return result;
}

std::pair<int, int> SimpleTournament::getSafeMove(AIAgentBase* agent, const Board& board, 
                                                CellState player, const std::string& agentName,
                                                bool& timedOut, bool& crashed) {
    timedOut = false;
    crashed = false;
    
    try {
        auto startTime = std::chrono::steady_clock::now();
        auto move = agent->getBestMove(board, player, config_.timeLimit);
        auto endTime = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        if (duration > config_.timeLimit) {
            timedOut = true;
            return {-1, -1};  // Invalid move to trigger loss
        }
        
        return move;
        
    } catch (const std::exception& e) {
        crashed = true;
        std::cerr << "Agent " << agentName << " crashed: " << e.what() << std::endl;
        return {-1, -1};  // Invalid move to trigger loss
    } catch (...) {
        crashed = true;
        std::cerr << "Agent " << agentName << " crashed with unknown exception" << std::endl;
        return {-1, -1};  // Invalid move to trigger loss
    }
}

void SimpleTournament::updateStats(const GameResult& result) {
    // Update black agent stats
    auto& blackStats = agentStats_[result.blackAgent];
    blackStats.gamesPlayed++;
    blackStats.totalScore += result.blackScore;
    blackStats.totalScoreAgainst += result.whiteScore;
    
    if (result.winner == "BLACK") {
        blackStats.wins++;
        blackStats.winsAgainst[result.whiteAgent]++;
    } else if (result.winner == "WHITE") {
        blackStats.losses++;
        blackStats.lossesAgainst[result.whiteAgent]++;
    } else {
        blackStats.draws++;
        blackStats.drawsAgainst[result.whiteAgent]++;
    }
    
    if (result.blackTimedOut) blackStats.timeouts++;
    if (result.blackCrashed) blackStats.crashes++;
    
    // Update white agent stats
    auto& whiteStats = agentStats_[result.whiteAgent];
    whiteStats.gamesPlayed++;
    whiteStats.totalScore += result.whiteScore;
    whiteStats.totalScoreAgainst += result.blackScore;
    
    if (result.winner == "WHITE") {
        whiteStats.wins++;
        whiteStats.winsAgainst[result.blackAgent]++;
    } else if (result.winner == "BLACK") {
        whiteStats.losses++;
        whiteStats.lossesAgainst[result.blackAgent]++;
    } else {
        whiteStats.draws++;
        whiteStats.drawsAgainst[result.blackAgent]++;
    }
    
    if (result.whiteTimedOut) whiteStats.timeouts++;
    if (result.whiteCrashed) whiteStats.crashes++;
    
    // Update win rates and averages
    for (auto& [name, stats] : agentStats_) {
        if (stats.gamesPlayed > 0) {
            stats.winRate = static_cast<double>(stats.wins) / stats.gamesPlayed;
            stats.averageScore = static_cast<double>(stats.totalScore) / stats.gamesPlayed;
            stats.averageScoreAgainst = static_cast<double>(stats.totalScoreAgainst) / stats.gamesPlayed;
        }
    }
}

void SimpleTournament::printProgress(int current, int total, const std::string& currentMatch) const {
    if (!config_.enableVisualFeedback) return;
    
    double progress = static_cast<double>(current) / total;
    int barWidth = 30;
    int pos = static_cast<int>(barWidth * progress);
    
    std::cout << "\rProgress: [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0) << "% ";
    std::cout << "(" << current << "/" << total << ") " << currentMatch;
    std::cout.flush();
}

void SimpleTournament::printResults() const {
    std::cout << "\n\nTOURNAMENT RESULTS\n";
    std::cout << std::string(80, '=') << std::endl;
    
    auto rankedResults = getRankedResults();
    
    std::cout << std::left << std::setw(15) << "Agent" 
              << std::setw(6) << "Games" 
              << std::setw(6) << "Wins" 
              << std::setw(6) << "Loss" 
              << std::setw(6) << "Draw"
              << std::setw(8) << "Win%"
              << std::setw(8) << "AvgScore"
              << std::setw(6) << "TO"
              << std::setw(6) << "Crash"
              << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& stats : rankedResults) {
        std::cout << std::left << std::setw(15) << stats.agentName
                  << std::setw(6) << stats.gamesPlayed
                  << std::setw(6) << stats.wins
                  << std::setw(6) << stats.losses
                  << std::setw(6) << stats.draws
                  << std::setw(8) << std::fixed << std::setprecision(1) << (stats.winRate * 100.0)
                  << std::setw(8) << std::fixed << std::setprecision(1) << stats.averageScore
                  << std::setw(6) << stats.timeouts
                  << std::setw(6) << stats.crashes
                  << std::endl;
    }
    
    std::cout << "\nLegend: TO = Timeouts, Crash = Crashes\n";
}

std::vector<AgentStats> SimpleTournament::getRankedResults() const {
    std::vector<AgentStats> results;
    for (const auto& [name, stats] : agentStats_) {
        results.push_back(stats);
    }
    
    // Sort by win rate, then by average score
    std::sort(results.begin(), results.end(), [](const AgentStats& a, const AgentStats& b) {
        if (std::abs(a.winRate - b.winRate) > 0.001) {
            return a.winRate > b.winRate;
        }
        return a.averageScore > b.averageScore;
    });
    
    return results;
}

void SimpleTournament::saveResults(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }
    
    file << "Othello Tournament Results\n";
    file << "==========================\n\n";
    
    auto rankedResults = getRankedResults();
    
    file << "Final Rankings:\n";
    file << std::left << std::setw(15) << "Agent" 
         << std::setw(6) << "Games" 
         << std::setw(6) << "Wins" 
         << std::setw(6) << "Loss" 
         << std::setw(6) << "Draw"
         << std::setw(8) << "Win%"
         << std::setw(8) << "AvgScore"
         << std::setw(6) << "TO"
         << std::setw(6) << "Crash"
         << std::endl;
    file << std::string(80, '-') << std::endl;
    
    for (const auto& stats : rankedResults) {
        file << std::left << std::setw(15) << stats.agentName
             << std::setw(6) << stats.gamesPlayed
             << std::setw(6) << stats.wins
             << std::setw(6) << stats.losses
             << std::setw(6) << stats.draws
             << std::setw(8) << std::fixed << std::setprecision(1) << (stats.winRate * 100.0)
             << std::setw(8) << std::fixed << std::setprecision(1) << stats.averageScore
             << std::setw(6) << stats.timeouts
             << std::setw(6) << stats.crashes
             << std::endl;
    }
    
    file << "\n\nIndividual Game Results:\n";
    file << "=======================\n";
    
    for (size_t i = 0; i < gameResults_.size(); ++i) {
        const auto& result = gameResults_[i];
        file << "Game " << (i + 1) << ": " << result.blackAgent << " vs " << result.whiteAgent << "\n";
        file << "  Result: " << result.winner << " (" << result.blackScore << "-" << result.whiteScore << ")\n";
        file << "  Moves: " << result.moveCount << ", Duration: " << formatDuration(result.gameDuration) << "\n";
        if (!result.failureReason.empty()) {
            file << "  Failure: " << result.failureReason << "\n";
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Results saved to " << filename << std::endl;
}

void SimpleTournament::saveResultsJson(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }

    file << "{\n";
    // Config
    file << "  \"config\": {\n";
    file << "    \"timeLimitMs\": " << config_.timeLimit.count() << ",\n";
    file << "    \"roundsPerMatchup\": " << config_.roundsPerMatchup << ",\n";
    file << "    \"tournamentType\": \"" << config_.tournamentType << "\"\n";
    file << "  },\n";

    // Agents
    file << "  \"agents\": [";
    for (size_t i = 0; i < agents_.size(); ++i) {
        file << "\"" << agents_[i]->getName() << "\"";
        if (i + 1 < agents_.size()) file << ", ";
    }
    file << "],\n";

    // Ranked stats
    auto ranked = getRankedResults();
    file << "  \"rankings\": [\n";
    for (size_t i = 0; i < ranked.size(); ++i) {
        const auto& s = ranked[i];
        file << "    {\n";
        file << "      \"agent\": \"" << s.agentName << "\",\n";
        file << "      \"games\": " << s.gamesPlayed << ",\n";
        file << "      \"wins\": " << s.wins << ",\n";
        file << "      \"losses\": " << s.losses << ",\n";
        file << "      \"draws\": " << s.draws << ",\n";
        file << "      \"winRate\": " << s.winRate << ",\n";
        file << "      \"avgScore\": " << s.averageScore << ",\n";
        file << "      \"timeouts\": " << s.timeouts << ",\n";
        file << "      \"crashes\": " << s.crashes << "\n";
        file << "    }" << (i + 1 < ranked.size() ? "," : "") << "\n";
    }
    file << "  ],\n";

    // Games
    file << "  \"games\": [\n";
    for (size_t i = 0; i < gameResults_.size(); ++i) {
        const auto& r = gameResults_[i];
        file << "    {\n";
        file << "      \"black\": \"" << r.blackAgent << "\",\n";
        file << "      \"white\": \"" << r.whiteAgent << "\",\n";
        file << "      \"blackScore\": " << r.blackScore << ",\n";
        file << "      \"whiteScore\": " << r.whiteScore << ",\n";
        file << "      \"winner\": \"" << r.winner << "\",\n";
        file << "      \"moves\": " << r.moveCount << ",\n";
        file << "      \"durationMs\": " << r.gameDuration.count() << ",\n";
        file << "      \"failureReason\": \"" << r.failureReason << "\",\n";
        file << "      \"blackTimedOut\": " << (r.blackTimedOut ? "true" : "false") << ",\n";
        file << "      \"whiteTimedOut\": " << (r.whiteTimedOut ? "true" : "false") << ",\n";
        file << "      \"blackCrashed\": " << (r.blackCrashed ? "true" : "false") << ",\n";
        file << "      \"whiteCrashed\": " << (r.whiteCrashed ? "true" : "false") << "\n";
        file << "    }" << (i + 1 < gameResults_.size() ? "," : "") << "\n";
    }
    file << "  ]\n";

    file << "}\n";
    file.close();
    std::cout << "Results saved to " << filename << std::endl;
}

void SimpleTournament::resetTournament() {
    gameResults_.clear();
    agentStats_.clear();
}

std::string SimpleTournament::formatDuration(std::chrono::milliseconds duration) const {
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto ms = duration % std::chrono::seconds(1);
    
    std::ostringstream oss;
    oss << seconds.count() << "." << std::setfill('0') << std::setw(3) << ms.count() << "s";
    return oss.str();
}

std::vector<std::string> SimpleTournament::getAgentNames() const {
    std::vector<std::string> names;
    for (const auto& agent : agents_) {
        names.push_back(agent->getName());
    }
    return names;
}
