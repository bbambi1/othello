#include "tournament_manager.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>

TournamentManager::TournamentManager() 
    : roundsPerMatchup(1), 
      timeLimit(std::chrono::milliseconds(5000)), 
      logGames(true), 
      logFilename("tournament.log"),
      totalGames(0) {
}

void TournamentManager::addAgent(std::unique_ptr<AIAgentBase> agent) {
    if (agent) {
        agents.push_back(std::move(agent));
    }
}

void TournamentManager::addAgent(const std::string& type, const std::string& name) {
    auto agent = createAIAgent(type, name.empty() ? type : name);
    if (agent) {
        agents.push_back(std::move(agent));
    }
}

void TournamentManager::addAgentWithSafety(std::unique_ptr<AIAgentBase> agent, const SafetyConfig& config) {
    if (agent) {
        // Wrap the agent with safety checks
        auto safeAgent = std::make_unique<SafeAIAgent>(std::move(agent), config);
        agents.push_back(std::move(safeAgent));
    }
}

void TournamentManager::runRoundRobin(int rounds) {
    if (agents.size() < 2) {
        std::cout << "Need at least 2 agents for a tournament" << std::endl;
        return;
    }
    
    std::cout << "Starting Round Robin Tournament with " << agents.size() << " agents" << std::endl;
    std::cout << rounds << " rounds per matchup" << std::endl;
    
    runRoundRobinInternal(rounds);
    updateTournamentResults();
}

void TournamentManager::runSingleElimination() {
    if (agents.size() < 2) {
        std::cout << "Need at least 2 agents for a tournament" << std::endl;
        return;
    }
    
    std::cout << "Starting Single Elimination Tournament with " << agents.size() << " agents" << std::endl;
    
    runSingleEliminationInternal();
    updateTournamentResults();
}

void TournamentManager::runSwissSystem(int rounds) {
    if (agents.size() < 2) {
        std::cout << "Need at least 2 agents for a tournament" << std::endl;
        return;
    }
    
    std::cout << "Starting Swiss System Tournament with " << agents.size() << " agents" << std::endl;
    std::cout << rounds << " rounds" << std::endl;
    
    runSwissSystemInternal(rounds);
    updateTournamentResults();
}

void TournamentManager::runRoundRobinInternal(int rounds) {
    int totalMatchups = (agents.size() * (agents.size() - 1)) / 2;
    int totalGames = totalMatchups * rounds;
    int currentGame = 0;
    
    for (size_t i = 0; i < agents.size(); ++i) {
        for (size_t j = i + 1; j < agents.size(); ++j) {
            std::cout << "\nMatchup: " << agents[i]->getName() << " vs " << agents[j]->getName() << std::endl;
            
            for (int round = 0; round < rounds; ++round) {
                currentGame++;
                printProgressBar(currentGame, totalGames);
                
                // Play game with agent i as black
                auto result1 = playGame(agents[i].get(), agents[j].get());
                gameResults.push_back(result1);
                
                // Play game with agent j as black (if rounds > 1)
                if (rounds > 1) {
                    currentGame++;
                    printProgressBar(currentGame, totalGames);
                    
                    auto result2 = playGame(agents[j].get(), agents[i].get());
                    gameResults.push_back(result2);
                }
            }
        }
    }
    
    std::cout << "\nRound Robin tournament completed!" << std::endl;
}

void TournamentManager::runSingleEliminationInternal() {
    // Simple single elimination - agents play until one remains
    std::vector<size_t> activeAgents;
    for (size_t i = 0; i < agents.size(); ++i) {
        activeAgents.push_back(i);
    }
    
    int round = 1;
    while (activeAgents.size() > 1) {
        std::cout << "\nRound " << round << " - " << activeAgents.size() << " agents remaining" << std::endl;
        
        std::vector<size_t> winners;
        for (size_t i = 0; i < activeAgents.size(); i += 2) {
            if (i + 1 < activeAgents.size()) {
                // Play game between two agents
                auto result = playGame(agents[activeAgents[i]].get(), agents[activeAgents[i + 1]].get());
                gameResults.push_back(result);
                
                // Determine winner
                size_t winnerIndex;
                if (result.winner == result.blackAgent) {
                    winnerIndex = activeAgents[i];
                } else {
                    winnerIndex = activeAgents[i + 1];
                }
                winners.push_back(winnerIndex);
                
                std::cout << agents[activeAgents[i]]->getName() << " vs " 
                         << agents[activeAgents[i + 1]]->getName() 
                         << " -> " << agents[winnerIndex]->getName() << " wins" << std::endl;
            } else {
                // Bye - agent advances automatically
                winners.push_back(activeAgents[i]);
            }
        }
        
        activeAgents = winners;
        round++;
    }
    
    if (!activeAgents.empty()) {
        std::cout << "\nTournament Winner: " << agents[activeAgents[0]]->getName() << std::endl;
    }
}

void TournamentManager::runSwissSystemInternal(int rounds) {
    std::vector<int> scores(agents.size(), 0);
    
    for (int round = 1; round <= rounds; ++round) {
        std::cout << "\nRound " << round << std::endl;
        
        // Generate pairings
        auto pairings = generateSwissPairings(scores);
        
        for (const auto& pairing : pairings) {
            int agent1 = pairing.first;
            int agent2 = pairing.second;
            
            std::cout << agents[agent1]->getName() << " vs " << agents[agent2]->getName() << std::endl;
            
            auto result = playGame(agents[agent1].get(), agents[agent2].get());
            gameResults.push_back(result);
            
            // Update scores
            if (result.winner == result.blackAgent) {
                scores[agent1] += 1;
            } else if (result.winner == result.whiteAgent) {
                scores[agent2] += 1;
            } else {
                // Draw
                scores[agent1] += 0.5;
                scores[agent2] += 0.5;
            }
        }
        
        // Print current standings
        std::cout << "\nStandings after round " << round << ":" << std::endl;
        std::vector<std::pair<int, int>> standings;
        for (size_t i = 0; i < agents.size(); ++i) {
            standings.push_back({i, scores[i]});
        }
        std::sort(standings.begin(), standings.end(), 
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        for (size_t i = 0; i < standings.size(); ++i) {
            std::cout << (i + 1) << ". " << agents[standings[i].first]->getName() 
                     << " - " << standings[i].second << " points" << std::endl;
        }
    }
}

GameResult TournamentManager::playGame(AIAgentBase* blackAgent, AIAgentBase* whiteAgent) {
    Board board;
    board.reset();
    
    GameResult result;
    result.blackAgent = blackAgent->getName();
    result.whiteAgent = whiteAgent->getName();
    result.moveCount = 0;
    result.gameLog = "";
    
    // Notify agents of game start
    blackAgent->onGameStart();
    whiteAgent->onGameStart();
    
    CellState currentPlayer = CellState::BLACK;
    AIAgentBase* currentAgent = blackAgent;
    
    while (!board.isGameOver() && board.hasValidMoves(currentPlayer)) {
        result.moveCount++;
        
        // Get move from current agent
        auto move = currentAgent->getBestMove(board, currentPlayer);
        
        if (move.first >= 0 && move.second >= 0 && 
            board.isValidMove(move.first, move.second, currentPlayer)) {
            
            // Log the move
            std::string playerName = (currentPlayer == CellState::BLACK) ? "Black" : "White";
            result.gameLog += formatGameLog(board, result.moveCount, playerName, move.first, move.second);
            
            // Make the move
            board.makeMove(move.first, move.second, currentPlayer);
            
            // Notify agents
            blackAgent->onMoveMade(move.first, move.second, currentPlayer);
            whiteAgent->onMoveMade(move.first, move.second, currentPlayer);
            
            // Switch players
            currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
            currentAgent = (currentPlayer == CellState::BLACK) ? blackAgent : whiteAgent;
        } else {
            // Invalid move - game over
            break;
        }
    }
    
    // Determine final scores and winner
    result.blackScore = board.getScore(CellState::BLACK);
    result.whiteScore = board.getScore(CellState::WHITE);
    
    if (result.blackScore > result.whiteScore) {
        result.winner = result.blackAgent;
        blackAgent->onGameEnd(CellState::BLACK);
        whiteAgent->onGameEnd(CellState::WHITE);
    } else if (result.whiteScore > result.blackScore) {
        result.winner = result.whiteAgent;
        blackAgent->onGameEnd(CellState::BLACK);
        whiteAgent->onGameEnd(CellState::WHITE);
    } else {
        result.winner = "Draw";
        blackAgent->onGameEnd(CellState::EMPTY);
        whiteAgent->onGameEnd(CellState::EMPTY);
    }
    
    totalGames++;
    
    if (logGames) {
        logGame(result);
    }
    
    return result;
}

GameResult TournamentManager::playGame(const std::string& blackAgentType, const std::string& whiteAgentType) {
    AIAgentBase* blackAgent = nullptr;
    AIAgentBase* whiteAgent = nullptr;
    
    // Find agents by type
    for (const auto& agent : agents) {
        if (agent->getName() == blackAgentType) {
            blackAgent = agent.get();
        } else if (agent->getName() == whiteAgentType) {
            whiteAgent = agent.get();
        }
    }
    
    if (!blackAgent || !whiteAgent) {
        std::cerr << "Error: Could not find agents for game" << std::endl;
        return GameResult{};
    }
    
    return playGame(blackAgent, whiteAgent);
}

std::vector<TournamentResult> TournamentManager::getResults() const {
    return tournamentResults;
}

void TournamentManager::printResults() const {
    std::cout << "\n=== TOURNAMENT RESULTS ===" << std::endl;
    std::cout << std::setw(20) << "Agent" 
              << std::setw(8) << "Games" 
              << std::setw(6) << "Wins" 
              << std::setw(6) << "Losses" 
              << std::setw(6) << "Draws" 
              << std::setw(10) << "Win Rate" 
              << std::setw(12) << "Avg Score" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& result : tournamentResults) {
        std::cout << std::setw(20) << result.agentName
                  << std::setw(8) << result.gamesPlayed
                  << std::setw(6) << result.wins
                  << std::setw(6) << result.losses
                  << std::setw(6) << result.draws
                  << std::setw(10) << std::fixed << std::setprecision(3) << result.winRate
                  << std::setw(12) << std::fixed << std::setprecision(1) << result.averageScore << std::endl;
    }
    
    std::cout << "\nTotal Games: " << totalGames << std::endl;
    std::cout << "Average Game Length: " << std::fixed << std::setprecision(1) 
              << getAverageGameLength() << " moves" << std::endl;
}

void TournamentManager::saveResults(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    file << "Tournament Results\n";
    file << "==================\n\n";
    
    file << std::setw(20) << "Agent" 
         << std::setw(8) << "Games" 
         << std::setw(6) << "Wins" 
         << std::setw(6) << "Losses" 
         << std::setw(6) << "Draws" 
         << std::setw(10) << "Win Rate" 
         << std::setw(12) << "Avg Score" << "\n";
    file << std::string(80, '-') << "\n";
    
    for (const auto& result : tournamentResults) {
        file << std::setw(20) << result.agentName
             << std::setw(8) << result.gamesPlayed
             << std::setw(6) << result.wins
             << std::setw(6) << result.losses
             << std::setw(6) << result.draws
             << std::setw(10) << std::fixed << std::setprecision(3) << result.winRate
             << std::setw(12) << std::fixed << std::setprecision(1) << result.averageScore << "\n";
    }
    
    file << "\nTotal Games: " << totalGames << "\n";
    file << "Average Game Length: " << std::fixed << std::setprecision(1) 
         << getAverageGameLength() << " moves\n";
    
    file.close();
    std::cout << "Results saved to " << filename << std::endl;
}

void TournamentManager::saveGameLogs(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    for (const auto& game : gameResults) {
        file << "Game: " << game.blackAgent << " (Black) vs " << game.whiteAgent << " (White)\n";
        file << "Result: " << game.winner << " wins\n";
        file << "Score: " << game.blackScore << " - " << game.whiteScore << "\n";
        file << "Moves: " << game.moveCount << "\n";
        file << "Game Log:\n" << game.gameLog << "\n";
        file << std::string(50, '-') << "\n\n";
    }
    
    file.close();
    std::cout << "Game logs saved to " << filename << std::endl;
}

void TournamentManager::resetTournament() {
    gameResults.clear();
    tournamentResults.clear();
    totalGames = 0;
    
    for (auto& agent : agents) {
        agent->resetStats();
    }
}

double TournamentManager::getAverageGameLength() const {
    if (gameResults.empty()) return 0.0;
    
    int totalMoves = 0;
    for (const auto& game : gameResults) {
        totalMoves += game.moveCount;
    }
    
    return static_cast<double>(totalMoves) / gameResults.size();
}

void TournamentManager::updateTournamentResults() {
    tournamentResults.clear();
    
    for (const auto& agent : agents) {
        TournamentResult result;
        result.agentName = agent->getName();
        result.gamesPlayed = 0;
        result.wins = 0;
        result.losses = 0;
        result.draws = 0;
        
        // Calculate wins/losses based on actual game results, not agent internal tracking
        for (const auto& game : gameResults) {
            if (game.blackAgent == agent->getName()) {
                result.gamesPlayed++;
                if (game.winner == game.blackAgent) {
                    result.wins++;
                } else if (game.winner == game.whiteAgent) {
                    result.losses++;
                } else {
                    result.draws++;
                }
            } else if (game.whiteAgent == agent->getName()) {
                result.gamesPlayed++;
                if (game.winner == game.whiteAgent) {
                    result.wins++;
                } else if (game.winner == game.blackAgent) {
                    result.losses++;
                } else {
                    result.draws++;
                }
            }
        }
        
        result.winRate = (result.gamesPlayed > 0) ? static_cast<double>(result.wins) / result.gamesPlayed : 0.0;
        
        // Calculate average score
        int totalScore = 0;
        int gamesCounted = 0;
        for (const auto& game : gameResults) {
            if (game.blackAgent == agent->getName()) {
                totalScore += game.blackScore;
                gamesCounted++;
            } else if (game.whiteAgent == agent->getName()) {
                totalScore += game.whiteScore;
                gamesCounted++;
            }
        }
        
        result.totalScore = totalScore;
        result.averageScore = (gamesCounted > 0) ? static_cast<double>(totalScore) / gamesCounted : 0.0;
        
        tournamentResults.push_back(result);
    }
    
    // Sort by win rate, then by average score
    std::sort(tournamentResults.begin(), tournamentResults.end(),
              [](const auto& a, const auto& b) {
                  if (std::abs(a.winRate - b.winRate) < 0.001) {
                      return a.averageScore > b.averageScore;
                  }
                  return a.winRate > b.winRate;
              });
}

void TournamentManager::logGame(const GameResult& result) {
    if (logFilename.empty()) return;
    
    std::ofstream file(logFilename, std::ios::app);
    if (file.is_open()) {
        file << "Game: " << result.blackAgent << " vs " << result.whiteAgent << "\n";
        file << "Winner: " << result.winner << "\n";
        file << "Score: " << result.blackScore << " - " << result.whiteScore << "\n";
        file << "Moves: " << result.moveCount << "\n";
        file << std::string(30, '-') << "\n";
        file.close();
    }
}

std::string TournamentManager::formatGameLog(const Board& board, int moveNumber, 
                                           const std::string& player, int row, int col) const {
    std::stringstream ss;
    ss << "Move " << moveNumber << ": " << player << " plays at (" << row << ", " << col << ")\n";
    
    // Add board state if detailed logging is enabled
    // This could be expanded to show the board after each move
    
    return ss.str();
}

void TournamentManager::printProgressBar(int current, int total) const {
    int barWidth = 50;
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(barWidth * progress);
    
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << current << "/" << total << " (" 
              << static_cast<int>(progress * 100.0) << "%)" << std::endl;
}

std::vector<std::pair<int, int>> TournamentManager::generateSwissPairings(const std::vector<int>& scores) const {
    std::vector<std::pair<int, int>> pairings;
    std::vector<bool> paired(agents.size(), false);
    
    // Simple pairing algorithm - pair agents with similar scores
    for (size_t i = 0; i < agents.size(); ++i) {
        if (paired[i]) continue;
        
        // Find best available opponent
        int bestOpponent = -1;
        int bestScoreDiff = std::numeric_limits<int>::max();
        
        for (size_t j = i + 1; j < agents.size(); ++j) {
            if (paired[j]) continue;
            
            int scoreDiff = std::abs(scores[i] - scores[j]);
            if (scoreDiff < bestScoreDiff) {
                bestScoreDiff = scoreDiff;
                bestOpponent = j;
            }
        }
        
        if (bestOpponent != -1) {
            pairings.push_back({i, bestOpponent});
            paired[i] = paired[bestOpponent] = true;
        }
    }
    
    return pairings;
}

int TournamentManager::calculateSwissScore(int agentIndex) const {
    int score = 0;
    for (const auto& game : gameResults) {
        if (game.blackAgent == agents[agentIndex]->getName()) {
            if (game.winner == game.blackAgent) score += 1;
            else if (game.winner == "Draw") score += 0;
        } else if (game.whiteAgent == agents[agentIndex]->getName()) {
            if (game.winner == game.whiteAgent) score += 1;
            else if (game.winner == "Draw") score += 0;
        }
    }
    return score;
}

void TournamentManager::printSafetyViolations() const {
    std::cout << "\n=== SAFETY VIOLATIONS REPORT ===" << std::endl;
    
    for (const auto& agent : agents) {
        // Try to cast to SafeAIAgent to access safety statistics
        if (auto safeAgent = dynamic_cast<const SafeAIAgent*>(agent.get())) {
            std::cout << "\nAgent: " << safeAgent->getWrappedAgentName() << std::endl;
            std::cout << "  Time Limit Violations: " << safeAgent->getTimeLimitViolations() << std::endl;
            std::cout << "  Invalid Move Violations: " << safeAgent->getInvalidMoveViolations() << std::endl;
            std::cout << "  Crash Violations: " << safeAgent->getCrashViolations() << std::endl;
            std::cout << "  Resource Violations: " << safeAgent->getResourceViolations() << std::endl;
            
            if (safeAgent->isDisqualified()) {
                std::cout << "  STATUS: DISQUALIFIED - " << safeAgent->getDisqualificationReason() << std::endl;
            } else {
                std::cout << "  STATUS: ACTIVE" << std::endl;
            }
        } else {
            std::cout << "\nAgent: " << agent->getName() << " (No safety wrapper)" << std::endl;
        }
    }
}

std::vector<std::string> TournamentManager::getDisqualifiedAgents() const {
    std::vector<std::string> disqualified;
    
    for (const auto& agent : agents) {
        if (auto safeAgent = dynamic_cast<const SafeAIAgent*>(agent.get())) {
            if (safeAgent->isDisqualified()) {
                disqualified.push_back(safeAgent->getWrappedAgentName());
            }
        }
    }
    
    return disqualified;
}
