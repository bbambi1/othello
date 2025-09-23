#pragma once

#include "ai_agent_base.h"
#include "board.h"
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct GameResult {
  std::string blackAgent;
  std::string whiteAgent;
  int blackScore;
  int whiteScore;
  std::string winner; // "BLACK", "WHITE", or "DRAW"
  int moveCount;
  std::chrono::milliseconds gameDuration;
  std::string failureReason; // Empty if no failure, otherwise reason for loss
  bool blackTimedOut;
  bool whiteTimedOut;
  bool blackCrashed;
  bool whiteCrashed;

  GameResult()
      : blackScore(0), whiteScore(0), moveCount(0), gameDuration(0),
        blackTimedOut(false), whiteTimedOut(false), blackCrashed(false),
        whiteCrashed(false) {}
};

// Statistics for a single agent
struct AgentStats {
  std::string agentName;
  int gamesPlayed;
  int wins;
  int losses;
  int draws;
  int timeouts;
  int crashes;
  int totalScore;
  int totalScoreAgainst;
  double winRate;
  double averageScore;
  double averageScoreAgainst;

  // Head-to-head records
  std::map<std::string, int> winsAgainst;
  std::map<std::string, int> lossesAgainst;
  std::map<std::string, int> drawsAgainst;

  AgentStats()
      : gamesPlayed(0), wins(0), losses(0), draws(0), timeouts(0), crashes(0),
        totalScore(0), totalScoreAgainst(0), winRate(0.0), averageScore(0.0),
        averageScoreAgainst(0.0) {}
};

struct TournamentConfig {
  std::chrono::milliseconds timeLimit{5000};
  int roundsPerMatchup{1};
  bool enableVisualFeedback{true};
  bool logGames{true};
  std::string logFile{"tournament.log"};
  std::string tournamentType{"round_robin"};

  TournamentConfig() = default;
  TournamentConfig(std::chrono::milliseconds limit, int rounds = 1)
      : timeLimit(limit), roundsPerMatchup(rounds) {}
};

using ProgressCallback = std::function<void(int current, int total,
                                            const std::string &currentMatch)>;

class SimpleTournament {
public:
  SimpleTournament();
  ~SimpleTournament() = default;

  void setConfig(const TournamentConfig &config) { config_ = config; }
  const TournamentConfig &getConfig() const { return config_; }

  void addAgent(std::unique_ptr<AIAgentBase> agent);
  void addAgent(const std::string &type, const std::string &name = "");
  void clearAgents();

  void setProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
  }

  void runRoundRobin();

  const std::vector<GameResult> &getGameResults() const { return gameResults_; }
  const std::map<std::string, AgentStats> &getAgentStats() const {
    return agentStats_;
  }
  std::vector<AgentStats> getRankedResults() const;

  std::vector<std::string> getAgentNames() const;

  void printResults() const;
  void saveResults(const std::string &filename) const;
  void saveResultsJson(const std::string &filename) const;
  void resetTournament();

  GameResult playSingleGame(AIAgentBase *blackAgent, AIAgentBase *whiteAgent);

private:
  std::vector<std::unique_ptr<AIAgentBase>> agents_;
  std::vector<GameResult> gameResults_;
  std::map<std::string, AgentStats> agentStats_;
  TournamentConfig config_;
  ProgressCallback progressCallback_;

  void updateStats(const GameResult &result);
  void printProgress(int current, int total,
                     const std::string &currentMatch) const;
  std::string formatDuration(std::chrono::milliseconds duration) const;

  std::pair<int, int> getSafeMove(AIAgentBase *agent, const Board &board,
                                  CellState player,
                                  const std::string &agentName, bool &timedOut,
                                  bool &crashed);
};
