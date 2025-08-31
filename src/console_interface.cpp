#include "console_interface.h"
#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include <thread>
#include <fstream>

ConsoleInterface::ConsoleInterface() 
    : currentMode(GameMode::HUMAN_VS_HUMAN), 
      currentPlayer(CellState::BLACK), 
      gameRunning(false),
      aiDepth(6),
      aiMoveDelay(1000) {
    aiAgent = std::make_unique<AIAgent>(aiDepth);
}

void ConsoleInterface::run() {
    while (true) {
        clearScreen();
        std::cout << "=== OTHELLO/REVERSI GAME ===\n\n";
        
        selectGameMode();
        
        if (currentMode == GameMode::HUMAN_VS_HUMAN) {
            playHumanVsHuman();
        } else if (currentMode == GameMode::HUMAN_VS_AI) {
            playHumanVsAI();
        } else if (currentMode == GameMode::AI_VS_AI) {
            playAIVsAI();
        }
        
        std::cout << "\nPlay another game? (y/n): ";
        if (!getYesNoInput("Play another game?")) {
            break;
        }
    }
    
    std::cout << "Thanks for playing!\n";
}

void ConsoleInterface::selectGameMode() {
    std::cout << "Select game mode:\n";
    std::cout << "1. Human vs Human\n";
    std::cout << "2. Human vs AI\n";
    std::cout << "3. AI vs AI\n";
    std::cout << "Enter choice (1-3): ";
    
    int choice;
    while (!(std::cin >> choice) || choice < 1 || choice > 3) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid choice. Please enter 1, 2, or 3: ";
    }
    
    currentMode = static_cast<GameMode>(choice - 1);
    
    if (currentMode != GameMode::HUMAN_VS_HUMAN) {
        std::cout << "Enter AI search depth (1-10, default 6): ";
        std::cin >> aiDepth;
        if (aiDepth < 1 || aiDepth > 10) {
            aiDepth = 6;
        }
        aiAgent->setMaxDepth(aiDepth);
        
        if (currentMode == GameMode::AI_VS_AI) {
            std::cout << "Enter delay between moves in milliseconds (0-5000, default 1000): ";
            std::cin >> aiMoveDelay;
            if (aiMoveDelay < 0 || aiMoveDelay > 5000) {
                aiMoveDelay = 1000;
            }
            
            if (aiMoveDelay == 0) {
                std::cout << "Fast mode enabled - game will run without delays\n";
            }
        }
    }
}

void ConsoleInterface::playHumanVsHuman() {
    initializeGame();
    
    while (gameRunning) {
        clearScreen();
        displayBoard();
        displayScore();
        
        if (board.hasValidMoves(currentPlayer)) {
            std::cout << "\n" << getPlayerName(currentPlayer) << "'s turn\n";
            processTurn();
        } else {
            handlePass();
        }
    }
    
    displayGameOver();
}

void ConsoleInterface::playHumanVsAI() {
    initializeGame();
    
    while (gameRunning) {
        clearScreen();
        displayBoard();
        displayScore();
        
        if (board.hasValidMoves(currentPlayer)) {
            std::cout << "\n" << getPlayerName(currentPlayer) << "'s turn\n";
            
            if (currentPlayer == CellState::BLACK) {
                // Human player
                processTurn();
            } else {
                // AI player
                std::cout << "AI is thinking...\n";
                auto aiMove = aiAgent->getBestMove(board, currentPlayer);
                if (aiMove.first != -1) {
                    board.makeMove(aiMove.first, aiMove.second, currentPlayer);
                    std::cout << "AI placed at (" << aiMove.first << ", " << aiMove.second << ")\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
                switchPlayer();
            }
        } else {
            handlePass();
        }
    }
    
    displayGameOver();
}

void ConsoleInterface::playAIVsAI() {
    initializeGame();
    
    // Record start time for game duration
    auto startTime = std::chrono::steady_clock::now();
    
    // Open log file for move recording
    std::ofstream logFile("ai_vs_ai_game.log");
    if (logFile.is_open()) {
        logFile << "=== AI vs AI Game Log ===\n";
        logFile << "AI Depth: " << aiDepth << "\n";
        logFile << "Starting position:\n";
        logFile.close();
    }
    
    std::cout << "AI vs AI mode - Game will run automatically\n";
    std::cout << "Moves will be logged to 'ai_vs_ai_game.log'\n";
    std::cout << "Press Enter to start...\n";
    waitForKeyPress();
    
    int moveNumber = 1;
    while (gameRunning) {
        clearScreen();
        displayBoard();
        displayScore();
        
        if (board.hasValidMoves(currentPlayer)) {
            std::cout << "\nMove " << moveNumber << " - " << getPlayerName(currentPlayer) << "'s turn\n";
            std::cout << "AI is thinking...\n";
            
            auto aiMove = aiAgent->getBestMove(board, currentPlayer);
            if (aiMove.first != -1) {
                board.makeMove(aiMove.first, aiMove.second, currentPlayer);
                            std::cout << "AI placed at (" << aiMove.first << ", " << aiMove.second << ")\n";
            std::cout << "Score: Black " << board.getScore(CellState::BLACK) 
                     << " - White " << board.getScore(CellState::WHITE) << "\n";
            
            // Log the move and board state
                logFile.open("ai_vs_ai_game.log", std::ios::app);
                if (logFile.is_open()) {
                    logFile << "Move " << moveNumber << ": " << getPlayerName(currentPlayer) 
                           << " -> (" << aiMove.first << ", " << aiMove.second << ")\n";
                    logFile << "Score after move: Black " << board.getScore(CellState::BLACK) 
                           << " - White " << board.getScore(CellState::WHITE) << "\n";
                    logFile.close();
                    
                    // Log the board state
                    logFile.open("ai_vs_ai_game.log", std::ios::app);
                    logBoardState(logFile, moveNumber);
                    logFile.close();
                }
                
                moveNumber++;
            }
            
            switchPlayer();
            
            // Small delay for visualization (configurable)
            std::this_thread::sleep_for(std::chrono::milliseconds(aiMoveDelay));
        } else {
            handlePass();
        }
    }
    
    // Log final game state
    logFile.open("ai_vs_ai_game.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "\n=== Final Game State ===\n";
        logFile << "Final Score - Black: " << board.getScore(CellState::BLACK) 
               << "  White: " << board.getScore(CellState::WHITE) << "\n";
        if (board.getScore(CellState::BLACK) > board.getScore(CellState::WHITE)) {
            logFile << "Winner: Black\n";
        } else if (board.getScore(CellState::WHITE) > board.getScore(CellState::BLACK)) {
            logFile << "Winner: White\n";
        } else {
            logFile << "Result: Tie\n";
        }
        
        // Game statistics
        logFile << "\n=== Game Statistics ===\n";
        logFile << "Total moves played: " << (moveNumber - 1) << "\n";
        logFile << "AI search depth: " << aiDepth << "\n";
        logFile << "Move delay: " << aiMoveDelay << "ms\n";
        logFile << "Game duration: " << std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count() << " seconds\n";
        
        logFile.close();
    }
    
    displayGameOver();
}

void ConsoleInterface::initializeGame() {
    board.reset();
    currentPlayer = CellState::BLACK;
    gameRunning = true;
}

void ConsoleInterface::processTurn() {
    auto move = getHumanMove(currentPlayer);
    if (move.first != -1) {
        board.makeMove(move.first, move.second, currentPlayer);
        switchPlayer();
    }
}

void ConsoleInterface::switchPlayer() {
    currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
    
    // Check if game is over
    if (board.isGameOver()) {
        gameRunning = false;
    }
}

void ConsoleInterface::handlePass() {
    std::cout << getPlayerName(currentPlayer) << " has no valid moves. Passing turn.\n";
    switchPlayer();
    waitForKeyPress();
}

void ConsoleInterface::displayBoard() const {
    std::cout << "   ";
    for (int col = 0; col < Board::BOARD_SIZE; ++col) {
        std::cout << " " << col << " ";
    }
    std::cout << "\n";
    
    for (int row = 0; row < Board::BOARD_SIZE; ++row) {
        std::cout << " " << row << " ";
        for (int col = 0; col < Board::BOARD_SIZE; ++col) {
            std::cout << "[" << getCellSymbol(board.getCell(row, col)) << "]";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void ConsoleInterface::displayScore() const {
    int blackScore = board.getScore(CellState::BLACK);
    int whiteScore = board.getScore(CellState::WHITE);
    
    std::cout << "Score - Black: " << blackScore << "  White: " << whiteScore << "\n";
    std::cout << "Total discs: " << board.getTotalDiscs() << "\n\n";
}

void ConsoleInterface::displayValidMoves(CellState player) const {
    auto validMoves = board.getValidMoves(player);
    if (!validMoves.empty()) {
        std::cout << "Valid moves: ";
        for (const auto& move : validMoves) {
            std::cout << "(" << move.first << "," << move.second << ") ";
        }
        std::cout << "\n";
    }
}

void ConsoleInterface::displayGameOver() const {
    clearScreen();
    displayBoard();
    displayScore();
    
    if (board.isGameOver()) {
        int blackScore = board.getScore(CellState::BLACK);
        int whiteScore = board.getScore(CellState::WHITE);
        
        std::cout << "=== GAME OVER ===\n";
        if (blackScore > whiteScore) {
            std::cout << "Black wins!\n";
        } else if (whiteScore > blackScore) {
            std::cout << "White wins!\n";
        } else {
            std::cout << "It's a tie!\n";
        }
    }
    
    waitForKeyPress();
}

std::pair<int, int> ConsoleInterface::getHumanMove(CellState player) const {
    displayValidMoves(player);
    
    int row, col;
    std::cout << "Enter row and column (e.g., 3 4): ";
    
    while (!(std::cin >> row >> col)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter two numbers: ";
    }
    
    if (board.isValidMove(row, col, player)) {
        return {row, col};
    } else {
        std::cout << "Invalid move! Press Enter to try again.\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        waitForKeyPress();
        return {-1, -1};
    }
}

bool ConsoleInterface::getYesNoInput(const std::string& prompt) const {
    (void)prompt; // Suppress unused parameter warning
    std::string input;
    std::cin >> input;
    return (input == "y" || input == "Y" || input == "yes" || input == "Yes");
}

void ConsoleInterface::clearScreen() const {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void ConsoleInterface::waitForKeyPress() const {
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

std::string ConsoleInterface::getPlayerName(CellState player) const {
    return (player == CellState::BLACK) ? "Black" : "White";
}

char ConsoleInterface::getCellSymbol(CellState state) const {
    switch (state) {
        case CellState::BLACK: return 'B';
        case CellState::WHITE: return 'W';
        case CellState::EMPTY: return ' ';
        default: return '?';
    }
}

void ConsoleInterface::logBoardState(std::ofstream& logFile, int moveNumber) const {
    if (!logFile.is_open()) return;
    
    logFile << "Board state after move " << moveNumber << ":\n";
    for (int row = 0; row < Board::BOARD_SIZE; ++row) {
        logFile << "  ";
        for (int col = 0; col < Board::BOARD_SIZE; ++col) {
            char symbol = getCellSymbol(board.getCell(row, col));
            logFile << "[" << symbol << "]";
        }
        logFile << "\n";
    }
    logFile << "\n";
}
