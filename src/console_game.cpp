#include "console_game.h"
#include "bitboard_ai_agent.h"
#include "greedy_ai_agent.h"
#include "mcts_ai_agent.h"
#include "minmax_ai_agent.h"
#include "random_ai_agent.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

ConsoleGame::ConsoleGame()
    : currentMode(ConsoleGameMode::HUMAN_VS_HUMAN),
      selectedAIAgent(AIAgentType::RANDOM),
      selectedOpponentAIAgent(AIAgentType::RANDOM),
      currentPlayer(CellState::BLACK), gameRunning(false), passCount(0),
      aiTimeLimit(std::chrono::milliseconds(1000)) {}

ConsoleGame::~ConsoleGame() = default;

void ConsoleGame::run() {
  while (true) {
    clearScreen();
    selectGameMode();

    startGame();
  }
}

void ConsoleGame::selectGameMode() {
  clearScreen();
  std::cout << "Select Game Mode" << std::endl;
  std::cout << "================" << std::endl;
  std::cout << std::endl;
  std::cout << "1. Human vs Human" << std::endl;
  std::cout << "2. Human vs AI" << std::endl;
  std::cout << "3. AI vs AI" << std::endl;
  std::cout << std::endl;

  int choice = getMenuChoice(1, 3);

  switch (choice) {
  case 1:
    currentMode = ConsoleGameMode::HUMAN_VS_HUMAN;
    break;
  case 2:
    currentMode = ConsoleGameMode::HUMAN_VS_AI;
    break;
  case 3:
    currentMode = ConsoleGameMode::AI_VS_AI;
    break;
  }

  if (currentMode == ConsoleGameMode::HUMAN_VS_AI ||
      currentMode == ConsoleGameMode::AI_VS_AI) {
    selectAIAgent();
  }

  if (currentMode == ConsoleGameMode::AI_VS_AI) {
    selectOpponentAIAgent();
  }

  if (currentMode != ConsoleGameMode::HUMAN_VS_HUMAN) {
    configureAISettings();
  }

  std::cout << std::endl;
}

void ConsoleGame::selectAIAgent() {
  clearScreen();
  displayAIAgentMenu();

  int choice = getMenuChoice(1, 5);

  switch (choice) {
  case 1:
    selectedAIAgent = AIAgentType::RANDOM;
    break;
  case 2:
    selectedAIAgent = AIAgentType::GREEDY;
    break;
  case 3:
    selectedAIAgent = AIAgentType::MINMAX;
    break;
  case 4:
    selectedAIAgent = AIAgentType::BITBOARD;
    break;
  case 5:
    selectedAIAgent = AIAgentType::MCTS;
    break;
  }

  std::cout << "AI Agent set to: " << getAIAgentTypeString(selectedAIAgent)
            << std::endl;
}

void ConsoleGame::selectOpponentAIAgent() {
  clearScreen();
  std::cout << "Select Opponent AI Agent" << std::endl;
  std::cout << "========================" << std::endl;
  std::cout << std::endl;
  displayAIAgentMenu();

  int choice = getMenuChoice(1, 5);

  switch (choice) {
  case 1:
    selectedOpponentAIAgent = AIAgentType::RANDOM;
    break;
  case 2:
    selectedOpponentAIAgent = AIAgentType::GREEDY;
    break;
  case 3:
    selectedOpponentAIAgent = AIAgentType::MINMAX;
    break;
  case 4:
    selectedOpponentAIAgent = AIAgentType::BITBOARD;
    break;
  case 5:
    selectedOpponentAIAgent = AIAgentType::MCTS;
    break;
  }

  std::cout << "Opponent AI Agent set to: "
            << getAIAgentTypeString(selectedOpponentAIAgent) << std::endl;
}

void ConsoleGame::displayAIAgentMenu() {
  std::cout << "Select AI Agent" << std::endl;
  std::cout << "===============" << std::endl;
  std::cout << std::endl;
  std::cout << "1. Random AI" << std::endl;
  std::cout << "2. Greedy AI" << std::endl;
  std::cout << "3. MinMax AI" << std::endl;
  std::cout << "4. BitBoard AI" << std::endl;
  std::cout << "5. MCTS AI" << std::endl;
  std::cout << std::endl;
}

void ConsoleGame::configureAISettings() {
  clearScreen();
  std::cout << "Configure AI Settings" << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << std::endl;

  // Time limit
  std::cout << "Current AI Time Limit: " << aiTimeLimit.count() << "ms"
            << std::endl;
  int timeLimit = getIntInput(
      "Enter new time limit in milliseconds (100-10000): ", 100, 10000);
  aiTimeLimit = std::chrono::milliseconds(timeLimit);

  std::cout << std::endl;
}

void ConsoleGame::startGame() {
  initializeGame();

  while (gameRunning) {
    clearScreen();
    displayBoard();
    displayScore();
    displayCurrentPlayer();

    if (currentMode == ConsoleGameMode::HUMAN_VS_HUMAN ||
        (currentMode == ConsoleGameMode::HUMAN_VS_AI &&
         currentPlayer == CellState::BLACK)) {
      std::pair<int, int> move = getHumanMove();
      if (move.first == -1 && move.second == -1) {
        // Pass
        handlePass();
      } else {
        board.makeMove(move.first, move.second, currentPlayer);
        passCount = 0;
        switchPlayer();
      }
    } else {
      // AI player's turn
      AIAgentBase *currentAI = (currentPlayer == CellState::BLACK)
                                   ? aiAgent.get()
                                   : opponentAIAgent.get();

      if (currentAI) {
        auto start = std::chrono::high_resolution_clock::now();
        auto move = currentAI->getBestMove(board, currentPlayer, aiTimeLimit);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (duration > aiTimeLimit) {
          std::cout << currentAI->getName() << " timed out!" << std::endl;
          handlePass();
          passCount--;
        } else {
          if (move.first == -1 && move.second == -1) {
            handlePass();
          } else {
            board.makeMove(move.first, move.second, currentPlayer);
            passCount = 0;
            switchPlayer();
          }

          if (currentMode == ConsoleGameMode::AI_VS_AI) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
          }
        }
      }
    }

    // Check for game over
    if (passCount >= 2 || board.isFull()) {
      gameRunning = false;
      displayGameOver();
    }
  }
}

void ConsoleGame::initializeGame() {
  board.reset();
  currentPlayer = CellState::BLACK;
  gameRunning = true;
  passCount = 0;

  if (currentMode == ConsoleGameMode::HUMAN_VS_AI ||
      currentMode == ConsoleGameMode::AI_VS_AI) {
    aiAgent.reset(createAIAgent(selectedAIAgent, "AI Agent"));
  }

  if (currentMode == ConsoleGameMode::AI_VS_AI) {
    opponentAIAgent.reset(
        createAIAgent(selectedOpponentAIAgent, "Opponent AI"));
  }
}

void ConsoleGame::displayBoard() {
  std::cout << "   ";
  for (int col = 0; col < 8; ++col) {
    std::cout << " " << static_cast<char>('A' + col) << " ";
  }
  std::cout << std::endl;

  for (int row = 0; row < 8; ++row) {
    std::cout << " " << (row + 1) << " ";
    for (int col = 0; col < 8; ++col) {
      CellState cell = board.getCell(row, col);
      switch (cell) {
      case CellState::EMPTY:
        std::cout << " . ";
        break;
      case CellState::BLACK:
        std::cout << " B ";
        break;
      case CellState::WHITE:
        std::cout << " W ";
        break;
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void ConsoleGame::displayScore() {
  int blackCount = board.getScore(CellState::BLACK);
  int whiteCount = board.getScore(CellState::WHITE);

  std::cout << "Score - Black: " << blackCount << ", White: " << whiteCount
            << std::endl;
  std::cout << std::endl;
}

void ConsoleGame::displayCurrentPlayer() {
  std::cout << "Current Player: " << getPlayerName(currentPlayer);

  if (currentMode == ConsoleGameMode::HUMAN_VS_AI &&
      currentPlayer == CellState::WHITE) {
    std::cout << " (" << getAIAgentTypeString(selectedAIAgent) << ")";
  } else if (currentMode == ConsoleGameMode::AI_VS_AI) {
    AIAgentType aiType = (currentPlayer == CellState::BLACK)
                             ? selectedAIAgent
                             : selectedOpponentAIAgent;
    std::cout << " (" << getAIAgentTypeString(aiType) << ")";
  }

  std::cout << std::endl;
  std::cout << std::endl;
}

void ConsoleGame::displayGameOver() {
  clearScreen();
  displayBoard();
  displayScore();

  int blackCount = board.getScore(CellState::BLACK);
  int whiteCount = board.getScore(CellState::WHITE);

  std::cout << "GAME OVER!" << std::endl;
  std::cout << "==========" << std::endl;
  std::cout << std::endl;

  if (blackCount > whiteCount) {
    std::cout << "Black wins! (" << blackCount << " vs " << whiteCount << ")"
              << std::endl;
  } else if (whiteCount > blackCount) {
    std::cout << "White wins! (" << whiteCount << " vs " << blackCount << ")"
              << std::endl;
  } else {
    std::cout << "It's a tie! (" << blackCount << " vs " << whiteCount << ")"
              << std::endl;
  }

  std::cout << std::endl;
  waitForKeyPress();
}

void ConsoleGame::displayValidMoves() {
  std::vector<std::pair<int, int>> validMoves =
      board.getValidMoves(currentPlayer);

  if (validMoves.empty()) {
    std::cout << "No valid moves available. Press Enter to pass." << std::endl;
  } else {
    std::cout << "Valid moves: ";
    for (size_t i = 0; i < validMoves.size(); ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << static_cast<char>('A' + validMoves[i].second)
                << (validMoves[i].first + 1);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

std::pair<int, int> ConsoleGame::getHumanMove() {
  displayValidMoves();

  std::vector<std::pair<int, int>> validMoves =
      board.getValidMoves(currentPlayer);

  if (validMoves.empty()) {
    std::cout << "No valid moves. Press Enter to pass...";
    waitForKeyPress();
    return {-1, -1};
  }

  while (true) {
    std::string input = getStringInput("Enter move (e.g., A1) or 'pass': ");

    if (input == "pass" || input == "p") {
      return {-1, -1};
    }

    if (input.length() >= 2) {
      char colChar = std::toupper(input[0]);
      char rowChar = input[1];

      if (colChar >= 'A' && colChar <= 'H' && rowChar >= '1' &&
          rowChar <= '8') {
        int col = colChar - 'A';
        int row = rowChar - '1';

        if (isValidMove(row, col, currentPlayer)) {
          return {row, col};
        } else {
          std::cout << "Invalid move. Try again." << std::endl;
        }
      } else {
        std::cout << "Invalid format. Use format like A1, B2, etc."
                  << std::endl;
      }
    } else {
      std::cout << "Invalid input. Use format like A1, B2, etc." << std::endl;
    }
  }
}

bool ConsoleGame::isValidMove(int row, int col, CellState player) {
  std::vector<std::pair<int, int>> validMoves = board.getValidMoves(player);
  return std::find(validMoves.begin(), validMoves.end(),
                   std::make_pair(row, col)) != validMoves.end();
}

void ConsoleGame::processTurn() {
  // This is handled in startGame()
}

void ConsoleGame::switchPlayer() {
  currentPlayer =
      (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
}

void ConsoleGame::handlePass() {
  passCount++;
  switchPlayer();
  std::cout << getPlayerName((currentPlayer == CellState::BLACK)
                                 ? CellState::WHITE
                                 : CellState::BLACK)
            << " passes." << std::endl;
  waitForKeyPress();
}

void ConsoleGame::resetGame() { initializeGame(); }

void ConsoleGame::clearScreen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void ConsoleGame::waitForKeyPress() {
  std::cout << "Press Enter to continue...";
  std::string dummy;
  std::getline(std::cin, dummy);
  std::cout << std::endl;
}

std::string ConsoleGame::getPlayerName(CellState player) const {
  return (player == CellState::BLACK) ? "Black" : "White";
}

std::string ConsoleGame::getAIAgentTypeString(AIAgentType type) const {
  switch (type) {
  case AIAgentType::RANDOM:
    return "Random";
  case AIAgentType::GREEDY:
    return "Greedy";
  case AIAgentType::MINMAX:
    return "MinMax";
  case AIAgentType::BITBOARD:
    return "BitBoard";
  case AIAgentType::MCTS:
    return "MCTS";
  default:
    return "Unknown";
  }
}

AIAgentBase *ConsoleGame::createAIAgent(AIAgentType type,
                                        const std::string &name) {
  switch (type) {
  case AIAgentType::RANDOM:
    return new RandomAIAgent(name);
  case AIAgentType::GREEDY:
    return new GreedyAIAgent(name);
  case AIAgentType::MINMAX:
    return new MinMaxAIAgent(name);
  case AIAgentType::BITBOARD:
    return new BitBoardAIAgent(name, "System", 8);
  case AIAgentType::MCTS:
    return new MCTSAiAgent(name);
  default:
    return new RandomAIAgent(name);
  }
}

int ConsoleGame::getMenuChoice(int min, int max) {
  int choice;
  while (true) {
    std::cout << "Enter your choice (" << min << "-" << max << "): ";
    std::cin >> choice;

    if (std::cin.fail() || choice < min || choice > max) {
      std::cin.clear();
      std::cin.ignore(10000, '\n');
      std::cout << "Invalid choice. Please try again." << std::endl;
    } else {
      std::cin.ignore(10000, '\n');
      return choice;
    }
  }
}

std::string ConsoleGame::getStringInput(const std::string &prompt) {
  std::string input;
  std::cout << prompt;
  std::getline(std::cin, input);
  return input;
}

int ConsoleGame::getIntInput(const std::string &prompt, int min, int max) {
  int value;
  while (true) {
    std::cout << prompt;
    std::cin >> value;

    if (std::cin.fail() || value < min || value > max) {
      std::cin.clear();
      std::cin.ignore(10000, '\n');
      std::cout << "Invalid input. Please enter a number between " << min
                << " and " << max << "." << std::endl;
    } else {
      std::cin.ignore(10000, '\n');
      return value;
    }
  }
}

bool ConsoleGame::getYesNoInput(const std::string &prompt) {
  std::string input;
  while (true) {
    std::cout << prompt;
    std::getline(std::cin, input);

    // Convert to lowercase
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);

    if (input == "y" || input == "yes") {
      return true;
    } else if (input == "n" || input == "no") {
      return false;
    } else {
      std::cout << "Please enter 'y' or 'n'." << std::endl;
    }
  }
}

void ConsoleGame::displayHelp() {
  clearScreen();
  std::cout << "Othello Console Game Help" << std::endl;
  std::cout << "=========================" << std::endl;
  std::cout << std::endl;
  std::cout << "How to Play:" << std::endl;
  std::cout
      << "- Enter moves in format A1, B2, etc. (column letter, row number)"
      << std::endl;
  std::cout << "- Type 'pass' or 'p' to pass your turn" << std::endl;
  std::cout << "- The game ends when both players pass or the board is full"
            << std::endl;
  std::cout << "- The player with more discs wins" << std::endl;
  std::cout << std::endl;
  std::cout << "AI Agents:" << std::endl;
  std::cout << "- Random: Makes random valid moves" << std::endl;
  std::cout << "- Greedy: Always takes the move that flips the most discs"
            << std::endl;
  std::cout << "- MinMax: Uses minimax algorithm with lookahead" << std::endl;
  std::cout << "- BitBoard: Uses bitboard representation for efficiency"
            << std::endl;
  std::cout << std::endl;
  waitForKeyPress();
}
