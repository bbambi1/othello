#pragma once

#include "ai_agent_base.h"
#include "board.h"
#include <chrono>
#include <memory>
#include <string>

enum class ConsoleGameMode { HUMAN_VS_HUMAN, HUMAN_VS_AI, AI_VS_AI };

enum class AIAgentType { RANDOM, GREEDY, MINMAX, BITBOARD, MCTS, PANDA };

class ConsoleGame {
public:
  ConsoleGame();
  ~ConsoleGame();

  void run();

  void selectGameMode();
  void selectAIAgent();
  void selectOpponentAIAgent();

  void playHumanVsHuman();
  void playHumanVsAI();
  void playAIVsAI();

  void displayBoard();
  void displayScore();
  void displayCurrentPlayer();
  void displayGameOver();
  void displayValidMoves();

  std::pair<int, int> getHumanMove();
  bool isValidMove(int row, int col, CellState player);

  void initializeGame();
  void startGame();
  void processTurn();
  void switchPlayer();
  void handlePass();
  void resetGame();

  void configureAISettings();

private:
  Board board;
  std::unique_ptr<AIAgentBase> aiAgent;
  std::unique_ptr<AIAgentBase> opponentAIAgent;
  ConsoleGameMode currentMode;
  AIAgentType selectedAIAgent;
  AIAgentType selectedOpponentAIAgent;

  CellState currentPlayer;
  bool gameRunning;
  int passCount;

  std::chrono::milliseconds aiTimeLimit;

  void clearScreen();
  void waitForKeyPress();
  std::string getPlayerName(CellState player) const;
  std::string getAIAgentTypeString(AIAgentType type) const;
  AIAgentBase *createAIAgent(AIAgentType type, const std::string &name);
  void displayMenu();
  void displayAIAgentMenu();
  int getMenuChoice(int min, int max);
  std::string getStringInput(const std::string &prompt);
  int getIntInput(const std::string &prompt, int min, int max);
  bool getYesNoInput(const std::string &prompt);
  void displayHelp();
};
