#pragma once

#include "board.h"
#include "ai_agent_base.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

enum class GUIGameMode {
    HUMAN_VS_HUMAN,
    HUMAN_VS_AI,
    AI_VS_AI
};

enum class AIAgentType {
    RANDOM,
    GREEDY,
    MINMAX,
    BITBOARD
};

class GUIInterface {
public:
    GUIInterface();
    ~GUIInterface();

    // Main game loop
    void run();

    // Game mode selection
    void selectGameMode();

    // AI agent selection
    void selectAIAgent();
    void selectOpponentAIAgent();

    // Game modes
    void playHumanVsHuman();
    void playHumanVsAI();
    void playAIVsAI();

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

private:
    // SFML components
    sf::RenderWindow window;
    sf::Font font;

    // Game objects
    Board board;
    std::unique_ptr<AIAgentBase> aiAgent;
    std::unique_ptr<AIAgentBase> opponentAIAgent;
    GUIGameMode currentMode;
    AIAgentType selectedAIAgent;
    AIAgentType selectedOpponentAIAgent;

    // Game state
    CellState currentPlayer;
    bool gameRunning;
    bool gamePaused;

    // AI settings
    bool aiJustMoved;
    int aiDepth;
    int aiMoveDelay;

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
    std::string getAIAgentTypeString(AIAgentType type) const;
};
