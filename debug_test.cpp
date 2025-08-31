#include "ai_agent_base.h"
#include "board.h"
#include <iostream>

int main() {
    std::cout << "=== Debug Test ===" << std::endl;
    
    // Create agents
    auto randomAgent = createAIAgent("random", "Random");
    auto minmaxAgent = createAIAgent("minmax", "MinMax");
    
    if (!randomAgent || !minmaxAgent) {
        std::cout << "Failed to create agents!" << std::endl;
        return 1;
    }
    
    std::cout << "Created agents: " << randomAgent->getName() << " and " << minmaxAgent->getName() << std::endl;
    
    // Play a single game
    Board board;
    board.reset();
    
    std::cout << "Starting game..." << std::endl;
    
    CellState currentPlayer = CellState::BLACK;
    int moveCount = 0;
    
    while (!board.isGameOver() && board.hasValidMoves(currentPlayer)) {
        moveCount++;
        
        AIAgentBase* currentAgent = (currentPlayer == CellState::BLACK) ? randomAgent.get() : minmaxAgent.get();
        std::string playerName = (currentPlayer == CellState::BLACK) ? "Random (Black)" : "MinMax (White)";
        
        std::cout << "Move " << moveCount << ": " << playerName << " thinking..." << std::endl;
        
        auto move = currentAgent->getBestMove(board, currentPlayer);
        
        if (move.first >= 0 && move.second >= 0 && board.isValidMove(move.first, move.second, currentPlayer)) {
            std::cout << "  " << playerName << " plays at (" << move.first << ", " << move.second << ")" << std::endl;
            
            board.makeMove(move.first, move.second, currentPlayer);
            
            // Print current scores
            int blackScore = board.getScore(CellState::BLACK);
            int whiteScore = board.getScore(CellState::WHITE);
            std::cout << "  Score: Black " << blackScore << " - White " << whiteScore << std::endl;
            
            // Switch players
            currentPlayer = (currentPlayer == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
        } else {
            std::cout << "  Invalid move! Game over." << std::endl;
            break;
        }
    }
    
    // Final results
    int blackScore = board.getScore(CellState::BLACK);
    int whiteScore = board.getScore(CellState::WHITE);
    
    std::cout << "\n=== Game Over ===" << std::endl;
    std::cout << "Total moves: " << moveCount << std::endl;
    std::cout << "Final score: Black " << blackScore << " - White " << whiteScore << std::endl;
    
    if (blackScore > whiteScore) {
        std::cout << "Winner: Random (Black)" << std::endl;
    } else if (whiteScore > blackScore) {
        std::cout << "Winner: MinMax (White)" << std::endl;
    } else {
        std::cout << "It's a tie!" << std::endl;
    }
    
    // Test the win/loss logic
    std::cout << "\n=== Testing Win/Loss Logic ===" << std::endl;
    
    if (blackScore > whiteScore) {
        randomAgent->onGameEnd(CellState::BLACK);
        minmaxAgent->onGameEnd(CellState::WHITE);
        std::cout << "Random agent thinks it won: " << randomAgent->getGamesWon() << " wins, " << randomAgent->getGamesPlayed() << " games" << std::endl;
        std::cout << "MinMax agent thinks it lost: " << minmaxAgent->getGamesWon() << " wins, " << minmaxAgent->getGamesPlayed() << " games" << std::endl;
    } else if (whiteScore > blackScore) {
        randomAgent->onGameEnd(CellState::BLACK);
        minmaxAgent->onGameEnd(CellState::WHITE);
        std::cout << "Random agent thinks it lost: " << randomAgent->getGamesWon() << " wins, " << randomAgent->getGamesPlayed() << " games" << std::endl;
        std::cout << "MinMax agent thinks it won: " << minmaxAgent->getGamesWon() << " wins, " << minmaxAgent->getGamesPlayed() << " games" << std::endl;
    }
    
    return 0;
}
