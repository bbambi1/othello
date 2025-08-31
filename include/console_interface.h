#pragma once

#include "board.h"
#include "ai_agent_base.h"
#include <memory>

enum class GameMode {
    HUMAN_VS_HUMAN,
    HUMAN_VS_AI,
    AI_VS_AI
};

class ConsoleInterface {
public:
    ConsoleInterface();
    
    // Main game loop
    void run();
    
    // Game mode selection
    void selectGameMode();
    
    // Game modes
    void playHumanVsHuman();
    void playHumanVsAI();
    void playAIVsAI();
    
    // Display functions
    void displayBoard() const;
    void displayScore() const;
    void displayValidMoves(CellState player) const;
    void displayGameOver() const;
    
    // Input functions
    std::pair<int, int> getHumanMove(CellState player) const;
    bool getYesNoInput(const std::string& prompt) const;
    
    // Utility functions
    void clearScreen() const;
    void waitForKeyPress() const;
    void logBoardState(std::ofstream& logFile, int moveNumber) const;
    
private:
    Board board;
    std::unique_ptr<AIAgentBase> aiAgent;
    GameMode currentMode;
    
    // Game state
    CellState currentPlayer;
    bool gameRunning;
    
    // AI settings
    int aiDepth;
    int aiMoveDelay; // Delay in milliseconds between AI moves
    
    // Helper functions
    void initializeGame();
    void processTurn();
    void switchPlayer();
    void handlePass();
    std::string getPlayerName(CellState player) const;
    char getCellSymbol(CellState state) const;
};
