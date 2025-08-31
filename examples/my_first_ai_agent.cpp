#include "ai_agent_base.h"
#include <random>
#include <algorithm>

// This is a simple example AI agent that participants can use as a starting point
// It implements a basic strategy: prefer corners, then edges, then interior positions
class MyFirstAIAgent : public AIAgentBase {
public:
    MyFirstAIAgent(const std::string& name = "MyFirstAgent") : AIAgentBase(name) {
        // Initialize random number generator
        static std::random_device rd;
        static std::mt19937 gen(rd());
        rng = gen;
    }
    
    // This is the main method you must implement!
    // It should return the best move as (row, col) for the given board state and player
    std::pair<int, int> getBestMove(const Board& board, CellState player) override {
        auto validMoves = getValidMoves(board, player);
        if (validMoves.empty()) {
            return {-1, -1}; // No valid moves
        }
        
        // Strategy: Prefer corners, then edges, then interior positions
        std::vector<std::pair<double, std::pair<int, int>>> scoredMoves;
        
        for (const auto& move : validMoves) {
            double score = evaluateMove(board, move.first, move.second, player);
            scoredMoves.push_back({score, move});
        }
        
        // Sort by score (highest first)
        std::sort(scoredMoves.begin(), scoredMoves.end(), 
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        
        // Return the best move
        return scoredMoves[0].second;
    }
    
    // Optional: Override these for additional functionality
    void onGameStart() override {
        // Called when a new game starts
        // You could initialize game-specific data here
    }
    
    void onMoveMade(int row, int col, CellState player) override {
        // Called after each move is made
        // You could update internal state or learn from moves here
    }
    
    void onGameEnd(CellState winner) override {
        // Called when the game ends
        // You could analyze the game result or update statistics here
    }
    
private:
    std::mt19937 rng;
    
    // Evaluate how good a move is
    double evaluateMove(const Board& board, int row, int col, CellState player) const {
        double score = 0.0;
        
        // 1. Corner moves are very valuable
        if (isCorner(row, col)) {
            score += 100.0;
        }
        // 2. Edge moves are valuable
        else if (isEdge(row, col)) {
            score += 20.0;
        }
        // 3. Interior moves are less valuable
        else {
            score += 5.0;
        }
        
        // 4. Bonus for moves that flip many discs
        auto flippedDiscs = board.getFlippedDiscs(row, col, player);
        score += flippedDiscs.size() * 2.0;
        
        // 5. Penalty for moves that give opponent corner access
        if (givesOpponentCornerAccess(board, row, col, player)) {
            score -= 50.0;
        }
        
        // 6. Small random factor to break ties (optional)
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        score += dis(rng) * 0.1;
        
        return score;
    }
    
    // Check if a position is a corner
    bool isCorner(int row, int col) const {
        return (row == 0 || row == 7) && (col == 0 || col == 7);
    }
    
    // Check if a position is an edge (but not corner)
    bool isEdge(int row, int col) const {
        return (row == 0 || row == 7 || col == 0 || col == 7) && !isCorner(row, col);
    }
    
    // Check if a move gives the opponent access to a corner
    bool givesOpponentCornerAccess(const Board& board, int row, int col, CellState player) const {
        CellState opponent = getOpponent(player);
        
        // Check all 8 directions around the move
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                
                int nr = row + dr;
                int nc = col + dc;
                
                // If this position is a corner and empty, opponent could play there
                if (isCorner(nr, nc) && board.getCell(nr, nc) == CellState::EMPTY) {
                    // Check if opponent has a valid move to this corner
                    if (board.isValidMove(nr, nc, opponent)) {
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
    // You can add more helper methods here
    double evaluateBoardState(const Board& board, CellState player) const {
        // This could be used for more sophisticated evaluation
        double score = 0.0;
        
        // Use the built-in evaluation functions from AIAgentBase
        score += evaluateCornerControl(board, player);
        score += evaluateEdgeControl(board, player);
        score += evaluateMobility(board, player) * 10.0;
        
        return score;
    }
};

// Register your agent so it can be used in tournaments
REGISTER_AI_AGENT(MyFirstAIAgent, "myfirstagent")

// Alternative registration with a different name
// REGISTER_AI_AGENT(MyFirstAIAgent, "myagent")
