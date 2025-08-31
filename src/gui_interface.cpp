#include "gui_interface.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

GUIInterface::GUIInterface() 
    : currentMode(GUIGameMode::HUMAN_VS_HUMAN), 
      currentPlayer(CellState::BLACK), 
      gameRunning(false), 
      gamePaused(false),
      aiDepth(6),
      aiMoveDelay(500),
      aiJustMoved(false) {
    initializeSFML();
    loadFonts();
    setupUI();
    setupBoard();
}

GUIInterface::~GUIInterface() {
    if (window.isOpen()) {
        window.close();
    }
}

void GUIInterface::initializeSFML() {
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Othello/Reversi Game");
    window.setFramerateLimit(60);
}

void GUIInterface::loadFonts() {
    // Try to load a system font, fallback to default if not available
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        if (!font.loadFromFile("/System/Library/Fonts/Arial.ttf")) {
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                std::cerr << "Warning: Could not load font, using default" << std::endl;
            }
        }
    }
}

void GUIInterface::setupUI() {
    // Setup text elements
    scoreText.setFont(font);
    scoreText.setCharacterSize(16);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);
    
    currentPlayerText.setFont(font);
    currentPlayerText.setCharacterSize(18);
    currentPlayerText.setFillColor(sf::Color::White);
    currentPlayerText.setPosition(10, 40);
    
    gameModeText.setFont(font);
    gameModeText.setCharacterSize(14);
    gameModeText.setFillColor(sf::Color::Yellow);
    gameModeText.setPosition(10, 70);
    
    instructionText.setFont(font);
    instructionText.setCharacterSize(14);
    instructionText.setFillColor(sf::Color::Cyan);
    instructionText.setPosition(10, WINDOW_HEIGHT - 40);
}

void GUIInterface::setupBoard() {
    // Setup board background
    boardBackground.setSize(sf::Vector2f(BOARD_SIZE, BOARD_SIZE));
    boardBackground.setPosition(BOARD_OFFSET_X, BOARD_OFFSET_Y);
    boardBackground.setFillColor(sf::Color(34, 139, 34)); // Forest green
    
    // Setup disc template
    discTemplate.setRadius(CELL_SIZE / 2 - 2);
    discTemplate.setOrigin(CELL_SIZE / 2 - 2, CELL_SIZE / 2 - 2);
    
    // Setup cell highlight
    cellHighlight.setSize(sf::Vector2f(CELL_SIZE - 4, CELL_SIZE - 4));
    cellHighlight.setFillColor(sf::Color(255, 255, 0, 128)); // Semi-transparent yellow
}

void GUIInterface::run() {
    selectGameMode();
    
    while (window.isOpen()) {
        handleEvents();
        
        if (gameRunning && !gamePaused) {
            processTurn();
        }
        
        render();
    }
}

void GUIInterface::selectGameMode() {
    bool modeSelected = false;
    
    while (!modeSelected && window.isOpen()) {
        handleEvents();
        
        // Clear window
        window.clear(sf::Color(50, 50, 50));
        
        // Draw title
        sf::Text titleText;
        titleText.setFont(font);
        titleText.setString("Othello/Reversi Game");
        titleText.setCharacterSize(32);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2, 100);
        window.draw(titleText);
        
        // Draw mode options
        std::vector<std::string> modes = {
            "1. Human vs Human",
            "2. Human vs AI", 
            "3. AI vs AI",
            "4. Tournament Mode"
        };
        
        for (size_t i = 0; i < modes.size(); ++i) {
            sf::Text modeText;
            modeText.setFont(font);
            modeText.setString(modes[i]);
            modeText.setCharacterSize(20);
            modeText.setFillColor(sf::Color::White);
            modeText.setPosition(WINDOW_WIDTH / 2 - modeText.getGlobalBounds().width / 2, 200 + i * 40);
            window.draw(modeText);
        }
        
        // Draw instructions
        sf::Text instructionText;
        instructionText.setFont(font);
        instructionText.setString("Press 1-4 to select game mode");
        instructionText.setCharacterSize(16);
        instructionText.setFillColor(sf::Color::Yellow);
        instructionText.setPosition(WINDOW_WIDTH / 2 - instructionText.getGlobalBounds().width / 2, 400);
        window.draw(instructionText);
        
        window.display();
        
        // Check for key presses
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
            currentMode = GUIGameMode::HUMAN_VS_HUMAN;
            modeSelected = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
            currentMode = GUIGameMode::HUMAN_VS_AI;
            modeSelected = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
            currentMode = GUIGameMode::AI_VS_AI;
            modeSelected = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
            currentMode = GUIGameMode::TOURNAMENT_MODE;
            modeSelected = true;
        }
    }
    
    initializeGame();
}

void GUIInterface::initializeGame() {
    board.reset();
    currentPlayer = CellState::BLACK;
    gameRunning = true;
    gamePaused = false;
    
    std::cout << "Initializing game in mode: ";
    switch (currentMode) {
        case GUIGameMode::HUMAN_VS_HUMAN: std::cout << "Human vs Human"; break;
        case GUIGameMode::HUMAN_VS_AI: std::cout << "Human vs AI"; break;
        case GUIGameMode::AI_VS_AI: std::cout << "AI vs AI"; break;
        case GUIGameMode::TOURNAMENT_MODE: std::cout << "Tournament"; break;
    }
    std::cout << std::endl;
    
    if (currentMode == GUIGameMode::HUMAN_VS_AI || 
        currentMode == GUIGameMode::AI_VS_AI) {
        aiAgent = std::make_unique<AIAgent>(aiDepth);
        std::cout << "AI Agent created with depth " << aiDepth << std::endl;
        std::cout << "AI Agent pointer: " << (aiAgent ? "valid" : "null") << std::endl;
    }
    
    if (currentMode == GUIGameMode::TOURNAMENT_MODE) {
        runTournament();
    }
}

void GUIInterface::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
            } else if (event.key.code == sf::Keyboard::Space) {
                gamePaused = !gamePaused;
            } else if (event.key.code == sf::Keyboard::R) {
                resetGame();
            }
        }
        
        if (event.type == sf::Event::MouseButtonPressed && 
            event.mouseButton.button == sf::Mouse::Left) {
            if (gameRunning && !gamePaused && 
                (currentMode == GUIGameMode::HUMAN_VS_HUMAN || 
                 (currentMode == GUIGameMode::HUMAN_VS_AI && currentPlayer == CellState::BLACK))) {
                
                auto mousePos = getMousePosition();
                if (isValidClick(mousePos.first, mousePos.second)) {
                    int row = (mousePos.second - BOARD_OFFSET_Y) / CELL_SIZE;
                    int col = (mousePos.first - BOARD_OFFSET_X) / CELL_SIZE;
                    
                    if (board.isValidMove(row, col, currentPlayer)) {
                        board.makeMove(row, col, currentPlayer);
                        animateMove(row, col, currentPlayer);
                        switchPlayer();
                    }
                }
            }
        }
    }
}

std::pair<int, int> GUIInterface::getMousePosition() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    return {mousePos.x, mousePos.y};
}

bool GUIInterface::isValidClick(int x, int y) {
    return x >= BOARD_OFFSET_X && x < BOARD_OFFSET_X + BOARD_SIZE &&
           y >= BOARD_OFFSET_Y && y < BOARD_OFFSET_Y + BOARD_SIZE;
}

void GUIInterface::render() {
    window.clear(sf::Color(50, 50, 50));
    
    renderBoard();
    renderUI();
    
    if (board.isGameOver()) {
        renderGameOver();
    }
    
    window.display();
}

void GUIInterface::renderBoard() {
    // Draw board background
    window.draw(boardBackground);
    
    // Draw grid lines
    for (int i = 0; i <= 8; ++i) {
        sf::RectangleShape line;
        line.setFillColor(sf::Color::Black);
        
        // Vertical lines
        line.setSize(sf::Vector2f(1, BOARD_SIZE));
        line.setPosition(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y);
        window.draw(line);
        
        // Horizontal lines
        line.setSize(sf::Vector2f(BOARD_SIZE, 1));
        line.setPosition(BOARD_OFFSET_X, BOARD_OFFSET_Y + i * CELL_SIZE);
        window.draw(line);
    }
    
    // Draw discs
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            CellState state = board.getCell(row, col);
            if (state != CellState::EMPTY) {
                drawDisc(row, col, state);
            }
        }
    }
    
    // Highlight valid moves for current player
    if (gameRunning && !gamePaused) {
        highlightValidMoves(currentPlayer);
    }
}

void GUIInterface::renderUI() {
    // Update and draw score
    std::stringstream scoreSS;
    scoreSS << "Black: " << board.getScore(CellState::BLACK) 
            << " | White: " << board.getScore(CellState::WHITE);
    scoreText.setString(scoreSS.str());
    window.draw(scoreText);
    
    // Update and draw current player
    std::stringstream playerSS;
    playerSS << "Current Player: " << getPlayerName(currentPlayer);
    currentPlayerText.setString(playerSS.str());
    currentPlayerText.setFillColor(getPlayerColor(currentPlayer));
    window.draw(currentPlayerText);
    
    // Draw game mode
    std::string modeStr;
    switch (currentMode) {
        case GUIGameMode::HUMAN_VS_HUMAN: modeStr = "Mode: Human vs Human"; break;
        case GUIGameMode::HUMAN_VS_AI: modeStr = "Mode: Human vs AI"; break;
        case GUIGameMode::AI_VS_AI: modeStr = "Mode: AI vs AI"; break;
        case GUIGameMode::TOURNAMENT_MODE: modeStr = "Mode: Tournament"; break;
    }
    gameModeText.setString(modeStr);
    window.draw(gameModeText);
    
    // Draw instructions
    std::string instructionStr = "Space: Pause | R: Reset | ESC: Exit";
    if (currentMode == GUIGameMode::HUMAN_VS_HUMAN || 
        (currentMode == GUIGameMode::HUMAN_VS_AI && currentPlayer == CellState::BLACK)) {
        instructionStr += " | Click to place disc";
    }
    instructionText.setString(instructionStr);
    window.draw(instructionText);
}

void GUIInterface::renderGameOver() {
    // Semi-transparent overlay
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);
    
    // Game over text
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(WINDOW_WIDTH / 2 - gameOverText.getGlobalBounds().width / 2, 
                           WINDOW_HEIGHT / 2 - 100);
    window.draw(gameOverText);
    
    // Winner text
    sf::Text winnerText;
    winnerText.setFont(font);
    winnerText.setCharacterSize(24);
    
    int blackScore = board.getScore(CellState::BLACK);
    int whiteScore = board.getScore(CellState::WHITE);
    
    if (blackScore > whiteScore) {
        winnerText.setString("Black Wins!");
        winnerText.setFillColor(sf::Color::Black);
    } else if (whiteScore > blackScore) {
        winnerText.setString("White Wins!");
        winnerText.setFillColor(sf::Color::White);
    } else {
        winnerText.setString("It's a Tie!");
        winnerText.setFillColor(sf::Color::Yellow);
    }
    
    winnerText.setPosition(WINDOW_WIDTH / 2 - winnerText.getGlobalBounds().width / 2, 
                          WINDOW_HEIGHT / 2 - 50);
    window.draw(winnerText);
    
    // Final score
    sf::Text finalScoreText;
    finalScoreText.setFont(font);
    finalScoreText.setString("Final Score - Black: " + std::to_string(blackScore) + 
                            " | White: " + std::to_string(whiteScore));
    finalScoreText.setCharacterSize(18);
    finalScoreText.setFillColor(sf::Color::White);
    finalScoreText.setPosition(WINDOW_WIDTH / 2 - finalScoreText.getGlobalBounds().width / 2, 
                              WINDOW_HEIGHT / 2);
    window.draw(finalScoreText);
    
    // Instructions
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString("Press R to play again or ESC to exit");
    restartText.setCharacterSize(16);
    restartText.setFillColor(sf::Color::Cyan);
    restartText.setPosition(WINDOW_WIDTH / 2 - restartText.getGlobalBounds().width / 2, 
                           WINDOW_HEIGHT / 2 + 50);
    window.draw(restartText);
}

void GUIInterface::drawDisc(int row, int col, CellState state) {
    sf::CircleShape disc = discTemplate;
    disc.setPosition(BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2,
                    BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2);
    
    if (state == CellState::BLACK) {
        disc.setFillColor(sf::Color::Black);
        disc.setOutlineColor(sf::Color::White);
    } else {
        disc.setFillColor(sf::Color::White);
        disc.setOutlineColor(sf::Color::Black);
    }
    disc.setOutlineThickness(2);
    
    window.draw(disc);
}

void GUIInterface::highlightValidMoves(CellState player) {
    auto validMoves = board.getValidMoves(player);
    for (const auto& move : validMoves) {
        sf::RectangleShape highlight = cellHighlight;
        highlight.setPosition(BOARD_OFFSET_X + move.second * CELL_SIZE + 2,
                            BOARD_OFFSET_Y + move.first * CELL_SIZE + 2);
        window.draw(highlight);
    }
}

void GUIInterface::animateMove(int row, int col, CellState player) {
    // Simple animation - could be enhanced with more sophisticated effects
    for (int i = 0; i < 5; ++i) {
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void GUIInterface::showGameOver() {
    gameRunning = false;
    // Game over rendering is handled in render()
}

std::string GUIInterface::getPlayerName(CellState player) const {
    return (player == CellState::BLACK) ? "Black" : "White";
}

sf::Color GUIInterface::getPlayerColor(CellState player) const {
    return (player == CellState::BLACK) ? sf::Color::Black : sf::Color::White;
}

void GUIInterface::switchPlayer() {
    currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
    
    // Check if current player has valid moves
    if (!board.hasValidMoves(currentPlayer)) {
        if (!board.hasValidMoves((currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK)) {
            // No valid moves for either player - game over
            showGameOver();
        } else {
            // Pass turn
            handlePass();
        }
    }
}

void GUIInterface::handlePass() {
    // Could add visual indication that turn was passed
    switchPlayer();
}

void GUIInterface::resetGame() {
    board.reset();
    currentPlayer = CellState::BLACK;
    gameRunning = true;
    gamePaused = false;
    aiJustMoved = false;
}

void GUIInterface::addAIAgent(const std::string& name, std::unique_ptr<AIAgent> agent) {
    tournamentAgents.push_back({name, std::move(agent), 0, 0, 0});
}

void GUIInterface::runTournament(int rounds) {
    if (tournamentAgents.size() < 2) {
        std::cout << "Need at least 2 AI agents for tournament" << std::endl;
        return;
    }
    
    std::cout << "Starting tournament with " << tournamentAgents.size() << " agents, " << rounds << " rounds per matchup" << std::endl;
    
    for (size_t i = 0; i < tournamentAgents.size(); ++i) {
        for (size_t j = i + 1; j < tournamentAgents.size(); ++j) {
            std::cout << "Match: " << tournamentAgents[i].name << " vs " << tournamentAgents[j].name << std::endl;
            
            for (int round = 0; round < rounds; ++round) {
                Board gameBoard;
                gameBoard.reset();
                CellState currentPlayer = CellState::BLACK;
                
                while (!gameBoard.isGameOver() && gameBoard.hasValidMoves(currentPlayer)) {
                    AIAgent* currentAgent = (currentPlayer == CellState::BLACK) ? 
                        tournamentAgents[i].agent.get() : tournamentAgents[j].agent.get();
                    
                    auto move = currentAgent->getBestMove(gameBoard, currentPlayer);
                    if (gameBoard.isValidMove(move.first, move.second, currentPlayer)) {
                        gameBoard.makeMove(move.first, move.second, currentPlayer);
                    }
                    
                    currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
                }
                
                // Determine winner
                int blackScore = gameBoard.getScore(CellState::BLACK);
                int whiteScore = gameBoard.getScore(CellState::WHITE);
                
                if (blackScore > whiteScore) {
                    tournamentAgents[i].wins++;
                    tournamentAgents[j].losses++;
                } else if (whiteScore > blackScore) {
                    tournamentAgents[j].wins++;
                    tournamentAgents[i].losses++;
                } else {
                    tournamentAgents[i].draws++;
                    tournamentAgents[j].draws++;
                }
            }
        }
    }
    
    displayTournamentResults();
    saveTournamentResults("tournament_results.txt");
}

void GUIInterface::displayTournamentResults() {
    std::cout << "\n=== TOURNAMENT RESULTS ===" << std::endl;
    for (const auto& agent : tournamentAgents) {
        std::cout << agent.name << ": " << agent.wins << "W " << agent.losses << "L " << agent.draws << "D" << std::endl;
    }
}

void GUIInterface::saveTournamentResults(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Tournament Results\n";
        file << "==================\n\n";
        for (const auto& agent : tournamentAgents) {
            file << agent.name << ": " << agent.wins << "W " << agent.losses << "L " << agent.draws << "D\n";
        }
        file.close();
        std::cout << "Results saved to " << filename << std::endl;
    }
}

// Game mode implementations
void GUIInterface::playHumanVsHuman() {
    currentMode = GUIGameMode::HUMAN_VS_HUMAN;
    initializeGame();
}

void GUIInterface::playHumanVsAI() {
    currentMode = GUIGameMode::HUMAN_VS_AI;
    initializeGame();
}

void GUIInterface::playAIVsAI() {
    currentMode = GUIGameMode::AI_VS_AI;
    initializeGame();
}

void GUIInterface::processTurn() {
    // Only process AI moves if it's an AI's turn
    if (currentMode == GUIGameMode::HUMAN_VS_HUMAN) {
        return; // Human vs Human mode - no AI processing needed
    }
    
    // If AI just moved, wait a bit before processing again
    if (aiJustMoved) {
        static auto lastMoveTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMoveTime);
        
        if (elapsed.count() < aiMoveDelay) {
            return; // Wait for the delay to complete
        }
        aiJustMoved = false;
        std::cout << "AI delay completed, ready for next move" << std::endl;
    }
    
    // Check if current player has valid moves
    if (!board.hasValidMoves(currentPlayer)) {
        if (!board.hasValidMoves((currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK)) {
            // No valid moves for either player - game over
            showGameOver();
            return;
        } else {
            // Pass turn
            std::cout << getPlayerName(currentPlayer) << " has no valid moves, passing turn" << std::endl;
            handlePass();
            return;
        }
    }
    
    // Handle AI moves
    if (currentMode == GUIGameMode::AI_VS_AI || 
        (currentMode == GUIGameMode::HUMAN_VS_AI && currentPlayer == CellState::WHITE)) {
        
        std::cout << "Processing AI turn for " << getPlayerName(currentPlayer) << std::endl;
        
        if (aiAgent && board.hasValidMoves(currentPlayer)) {
            std::cout << "AI (" << getPlayerName(currentPlayer) << ") thinking..." << std::endl;
            auto start = std::chrono::steady_clock::now();
            auto move = aiAgent->getBestMove(board, currentPlayer);
            auto end = std::chrono::steady_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "AI move: (" << move.first << ", " << move.second << ") in " << duration.count() << "ms" << std::endl;
            
            if (board.isValidMove(move.first, move.second, currentPlayer)) {
                board.makeMove(move.first, move.second, currentPlayer);
                std::cout << "AI made valid move at (" << move.first << ", " << move.second << ")" << std::endl;
                animateMove(move.first, move.second, currentPlayer);
                
                // Set flag to prevent rapid moves
                aiJustMoved = true;
                
                switchPlayer();
            } else {
                // Invalid move from AI - this shouldn't happen with proper AI
                std::cerr << "AI returned invalid move: (" << move.first << ", " << move.second << ")" << std::endl;
                switchPlayer();
            }
        } else if (!aiAgent) {
            std::cerr << "No AI agent available!" << std::endl;
        } else {
            std::cout << "AI has no valid moves available" << std::endl;
        }
    } else {
        std::cout << "Not AI's turn - current player: " << getPlayerName(currentPlayer) << std::endl;
    }
}
