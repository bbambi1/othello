#pragma once

#include "board.h"
#include "ai_agent.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

enum class GUIGameMode {
    HUMAN_VS_HUMAN,
    HUMAN_VS_AI,
    AI_VS_AI,
    TOURNAMENT_MODE
};

class GUIInterface {
public:
    GUIInterface();
    ~GUIInterface();
    
    // Main game loop
    void run();
    
    // Game mode selection
    void selectGameMode();
    
    // Game modes
    void playHumanVsHuman();
    void playHumanVsAI();
    void playAIVsAI();
    void runTournament(int rounds = 1);
    
    // Display functions
    void render();
    void renderBoard();
    void renderUI();
    void renderGameOver();
    
    // Input handling
    void handleEvents();
    std::pair<int, int> getMousePosition();
    bool isValidClick(int x, int y);
    
    // Game state management
    void initializeGame();
    void processTurn();
    void switchPlayer();
    void handlePass();
    void resetGame();
    
    // AI tournament functions
    void addAIAgent(const std::string& name, std::unique_ptr<AIAgent> agent);
    void runAITournament(int rounds);
    
private:
    // SFML components
    sf::RenderWindow window;
    sf::Font font;
    
    // Game objects
    Board board;
    std::unique_ptr<AIAgent> aiAgent;
    GUIGameMode currentMode;
    
    // Game state
    CellState currentPlayer;
    bool gameRunning;
    bool gamePaused;
    
    // AI settings
    int aiDepth;
    int aiMoveDelay;
    bool aiJustMoved;  // Flag to prevent AI from moving too quickly
    
    // Visual elements
    sf::RectangleShape boardBackground;
    sf::CircleShape discTemplate;
    sf::RectangleShape cellHighlight;
    
    // Colors and dimensions
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;
    static constexpr int BOARD_SIZE = 400;
    static constexpr int CELL_SIZE = BOARD_SIZE / 8;
    static constexpr int BOARD_OFFSET_X = (WINDOW_WIDTH - BOARD_SIZE) / 2;
    static constexpr int BOARD_OFFSET_Y = (WINDOW_HEIGHT - BOARD_SIZE) / 2;
    
    // UI elements
    sf::Text scoreText;
    sf::Text currentPlayerText;
    sf::Text gameModeText;
    sf::Text instructionText;
    
    // AI tournament
    struct TournamentAgent {
        std::string name;
        std::unique_ptr<AIAgent> agent;
        int wins;
        int losses;
        int draws;
    };
    std::vector<TournamentAgent> tournamentAgents;
    
    // Helper functions
    void initializeSFML();
    void loadFonts();
    void setupUI();
    void setupBoard();
    void drawDisc(int row, int col, CellState state);
    void highlightValidMoves(CellState player);
    void animateMove(int row, int col, CellState player);
    void showGameOver();
    std::string getPlayerName(CellState player) const;
    sf::Color getPlayerColor(CellState player) const;
    
    // Tournament functions
    void displayTournamentResults();
    void saveTournamentResults(const std::string& filename);
};
