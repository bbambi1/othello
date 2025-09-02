#include "gui_interface.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

GUIInterface::GUIInterface() 
    : currentMode(GUIGameMode::HUMAN_VS_HUMAN), 
      selectedAIAgent(AIAgentType::MINMAX),
      selectedOpponentAIAgent(AIAgentType::MINMAX),
      currentPlayer(CellState::BLACK), 
      gameRunning(false), 
      gamePaused(false),
      aiJustMoved(false),
      aiDepth(6),
      aiMoveDelay(500) {
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
            "3. AI vs AI"
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
        instructionText.setString("Press 1-3 to select game mode");
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
        }
    }
    
    // For AI modes, select AI agents before initializing the game
    if (currentMode == GUIGameMode::HUMAN_VS_AI || currentMode == GUIGameMode::AI_VS_AI) {
        std::cout << "About to select AI agent for mode: " << static_cast<int>(currentMode) << std::endl;
        selectAIAgent();
        std::cout << "AI agent selection completed. Selected: " << static_cast<int>(selectedAIAgent) << std::endl;
        
        // For AI vs AI mode, also select opponent AI
        if (currentMode == GUIGameMode::AI_VS_AI) {
            std::cout << "About to select opponent AI agent" << std::endl;
            selectOpponentAIAgent();
            std::cout << "Opponent AI agent selection completed. Selected: " << static_cast<int>(selectedOpponentAIAgent) << std::endl;
        }
    }
    
    initializeGame();
}

void GUIInterface::selectAIAgent() {
    bool agentSelected = false;
    
    std::cout << "Entering AI agent selection screen..." << std::endl;
    
    while (!agentSelected && window.isOpen()) {
        handleEvents();
        
        // Clear window
        window.clear(sf::Color(50, 50, 50));
        
        // Draw title
        sf::Text titleText;
        titleText.setFont(font);
        titleText.setString("Select AI Agent");
        titleText.setCharacterSize(32);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2, 100);
        window.draw(titleText);
        
        // Draw subtitle based on game mode
        sf::Text subtitleText;
        subtitleText.setFont(font);
        if (currentMode == GUIGameMode::HUMAN_VS_AI) {
            subtitleText.setString("(Your opponent)");
        } else {
            subtitleText.setString("(Black player)");
        }
        subtitleText.setCharacterSize(18);
        subtitleText.setFillColor(sf::Color::Yellow);
        subtitleText.setPosition(WINDOW_WIDTH / 2 - subtitleText.getGlobalBounds().width / 2, 140);
        window.draw(subtitleText);
        
        // Draw agent options
        std::vector<std::string> agents = {
            "A. Random AI",
            "B. Greedy AI", 
            "C. MinMax AI",
            "D. BitBoard AI"
        };
        
        for (size_t i = 0; i < agents.size(); ++i) {
            sf::Text agentText;
            agentText.setFont(font);
            agentText.setString(agents[i]);
            agentText.setCharacterSize(20);
            agentText.setFillColor(sf::Color::White);
            agentText.setPosition(WINDOW_WIDTH / 2 - agentText.getGlobalBounds().width / 2, 200 + i * 40);
            window.draw(agentText);
        }
        
        // Draw instructions
        sf::Text instructionText;
        instructionText.setFont(font);
        instructionText.setString("Press A-D to select AI agent");
        instructionText.setCharacterSize(16);
        instructionText.setFillColor(sf::Color::Yellow);
        instructionText.setPosition(WINDOW_WIDTH / 2 - instructionText.getGlobalBounds().width / 2, 400);
        window.draw(instructionText);
        
        window.display();

        // Check for key press events and mouse clicks
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }

            if (event.type == sf::Event::KeyPressed) {
                std::cout << "Key pressed: " << event.key.code << std::endl;

                if (event.key.code == sf::Keyboard::A) {
                    selectedAIAgent = AIAgentType::RANDOM;
                    std::cout << "Selected AI Agent: Random (A key)" << std::endl;
                    agentSelected = true;
                } else if (event.key.code == sf::Keyboard::B) {
                    selectedAIAgent = AIAgentType::GREEDY;
                    std::cout << "Selected AI Agent: Greedy (B key)" << std::endl;
                    agentSelected = true;
                } else if (event.key.code == sf::Keyboard::C) {
                    selectedAIAgent = AIAgentType::MINMAX;
                    std::cout << "Selected AI Agent: MinMax (C key)" << std::endl;
                    agentSelected = true;
                } else if (event.key.code == sf::Keyboard::D) {
                    selectedAIAgent = AIAgentType::BITBOARD;
                    std::cout << "Selected AI Agent: BitBoard (D key)" << std::endl;
                    agentSelected = true;
                } else {
                    std::cout << "Unknown key pressed: " << event.key.code << std::endl;
                }
            }
        }
    }
}

void GUIInterface::selectOpponentAIAgent() {
    bool agentSelected = false;
    
    std::cout << "Entering opponent AI agent selection screen..." << std::endl;
    
    while (!agentSelected && window.isOpen()) {
        handleEvents();
        
        // Clear window
        window.clear(sf::Color(50, 50, 50));
        
        // Draw title
        sf::Text titleText;
        titleText.setFont(font);
        titleText.setString("Select Opponent AI Agent");
        titleText.setCharacterSize(32);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2, 100);
        window.draw(titleText);
        
        // Draw subtitle
        sf::Text subtitleText;
        subtitleText.setFont(font);
        subtitleText.setString("(White player)");
        subtitleText.setCharacterSize(18);
        subtitleText.setFillColor(sf::Color::Yellow);
        subtitleText.setPosition(WINDOW_WIDTH / 2 - subtitleText.getGlobalBounds().width / 2, 140);
        window.draw(subtitleText);
        
        // Draw agent options
        std::vector<std::string> agents = {
            "A. Random AI",
            "B. Greedy AI", 
            "C. MinMax AI",
            "D. BitBoard AI"
        };
        
        for (size_t i = 0; i < agents.size(); ++i) {
            sf::Text agentText;
            agentText.setFont(font);
            agentText.setString(agents[i]);
            agentText.setCharacterSize(20);
            agentText.setFillColor(sf::Color::White);
            agentText.setPosition(WINDOW_WIDTH / 2 - agentText.getGlobalBounds().width / 2, 200 + i * 40);
            window.draw(agentText);
        }
        
        // Draw instructions
        sf::Text instructionText;
        instructionText.setFont(font);
        instructionText.setString("Press A-D to select opponent AI agent");
        instructionText.setCharacterSize(16);
        instructionText.setFillColor(sf::Color::Yellow);
        instructionText.setPosition(WINDOW_WIDTH / 2 - instructionText.getGlobalBounds().width / 2, 400);
        window.draw(instructionText);
        
        window.display();
        
        // Check for key press events and mouse clicks
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            
            if (event.type == sf::Event::KeyPressed) {
                std::cout << "Opponent AI - Key pressed: " << event.key.code << std::endl;

                if (event.key.code == sf::Keyboard::A) {
                    selectedOpponentAIAgent = AIAgentType::RANDOM;
                    std::cout << "Selected Opponent AI Agent: Random (A key)" << std::endl;
                    agentSelected = true;
                } else if (event.key.code == sf::Keyboard::B) {
                    selectedOpponentAIAgent = AIAgentType::GREEDY;
                    std::cout << "Selected Opponent AI Agent: Greedy (B key)" << std::endl;
                    agentSelected = true;
                } else if (event.key.code == sf::Keyboard::C) {
                    selectedOpponentAIAgent = AIAgentType::MINMAX;
                    std::cout << "Selected Opponent AI Agent: MinMax (C key)" << std::endl;
                    agentSelected = true;
                } else if (event.key.code == sf::Keyboard::D) {
                    selectedOpponentAIAgent = AIAgentType::BITBOARD;
                    std::cout << "Selected Opponent AI Agent: BitBoard (D key)" << std::endl;
                    agentSelected = true;
                } else {
                    std::cout << "Opponent AI - Unknown key pressed: " << event.key.code << std::endl;
                }
            }
        }
    }
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

    }
    std::cout << std::endl;
    
    if (currentMode == GUIGameMode::HUMAN_VS_AI || 
        currentMode == GUIGameMode::AI_VS_AI) {
        // Create AI agents based on selections made earlier
        std::cout << "Creating AI agent with selection: " << static_cast<int>(selectedAIAgent) << std::endl;
        std::string agentType = "";
        switch (selectedAIAgent) {
            case AIAgentType::RANDOM: agentType = "random"; break;
            case AIAgentType::GREEDY: agentType = "greedy"; break;
            case AIAgentType::MINMAX: agentType = "minmax"; break;
            case AIAgentType::BITBOARD: agentType = "bitboard"; break;
        }
        std::cout << "Agent type string: " << agentType << std::endl;
        aiAgent = createAIAgent(agentType, getAIAgentTypeString(selectedAIAgent));
        if (aiAgent) {
            std::cout << "AI Agent created: " << aiAgent->getName() << std::endl;
        } else {
            std::cout << "Failed to create AI Agent" << std::endl;
        }

        // For AI vs AI mode, also create opponent AI
        if (currentMode == GUIGameMode::AI_VS_AI) {
            std::cout << "Creating opponent AI agent with selection: " << static_cast<int>(selectedOpponentAIAgent) << std::endl;
            std::string opponentAgentType = "";
            switch (selectedOpponentAIAgent) {
                case AIAgentType::RANDOM: opponentAgentType = "random"; break;
                case AIAgentType::GREEDY: opponentAgentType = "greedy"; break;
                case AIAgentType::MINMAX: opponentAgentType = "minmax"; break;
                case AIAgentType::BITBOARD: opponentAgentType = "bitboard"; break;
            }
            std::cout << "Opponent agent type string: " << opponentAgentType << std::endl;
            opponentAIAgent = createAIAgent(opponentAgentType, getAIAgentTypeString(selectedOpponentAIAgent));
            if (opponentAIAgent) {
                std::cout << "Opponent AI Agent created: " << opponentAIAgent->getName() << std::endl;
            } else {
                std::cout << "Failed to create Opponent AI Agent" << std::endl;
            }
        }
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

    }
    gameModeText.setString(modeStr);
    window.draw(gameModeText);
    
    // Draw AI agent information
    if (currentMode == GUIGameMode::HUMAN_VS_AI || currentMode == GUIGameMode::AI_VS_AI) {
        sf::Text aiInfoText;
        aiInfoText.setFont(font);
        aiInfoText.setCharacterSize(12);
        aiInfoText.setFillColor(sf::Color::Cyan);
        
        if (currentMode == GUIGameMode::HUMAN_VS_AI) {
            aiInfoText.setString("AI Agent: " + getAIAgentTypeString(selectedAIAgent));
            aiInfoText.setPosition(10, 100);
        } else { // AI vs AI
            aiInfoText.setString("Black AI: " + getAIAgentTypeString(selectedAIAgent) + 
                                " | White AI: " + getAIAgentTypeString(selectedOpponentAIAgent));
            aiInfoText.setPosition(10, 100);
        }
        window.draw(aiInfoText);
    }
    
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

void GUIInterface::animateMove(int /*row*/, int /*col*/, CellState /*player*/) {
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
        
        // Determine which AI agent to use based on current player and game mode
        AIAgentBase* currentAIAgent = nullptr;
        if (currentMode == GUIGameMode::AI_VS_AI) {
            // In AI vs AI mode, use different agents for each player
            currentAIAgent = (currentPlayer == CellState::BLACK) ? aiAgent.get() : opponentAIAgent.get();
        } else {
            // In Human vs AI mode, always use the selected AI agent
            currentAIAgent = aiAgent.get();
        }
        
        if (currentAIAgent && board.hasValidMoves(currentPlayer)) {
            std::cout << "AI (" << getPlayerName(currentPlayer) << ") thinking..." << std::endl;
            auto start = std::chrono::steady_clock::now();
            // Set a 1 second time limit for AI moves
            auto move = currentAIAgent->getBestMove(board, currentPlayer, std::chrono::milliseconds(1000));
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
        } else if (!currentAIAgent) {
            std::cerr << "No AI agent available!" << std::endl;
        } else {
            std::cout << "AI has no valid moves available" << std::endl;
        }
    } else {
        std::cout << "Not AI's turn - current player: " << getPlayerName(currentPlayer) << std::endl;
    }
}

std::string GUIInterface::getAIAgentTypeString(AIAgentType type) const {
    switch (type) {
        case AIAgentType::RANDOM: return "Random";
        case AIAgentType::GREEDY: return "Greedy";
        case AIAgentType::MINMAX: return "MinMax";
        case AIAgentType::BITBOARD: return "BitBoard";
        default: return "Unknown";
    }
}
