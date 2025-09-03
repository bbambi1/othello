#include "greedy_ai_agent.h"
#include <algorithm>
#include <chrono>

GreedyAIAgent::GreedyAIAgent(const std::string& name, const std::string& author) 
    : AIAgentBase(name, author) {
}

std::pair<int, int> GreedyAIAgent::getBestMove(const Board& board, CellState player, 
                                               std::chrono::milliseconds timeLimit) {
    auto startTime = std::chrono::steady_clock::now();

    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1};
    }

    std::pair<int, int> bestMove = validMoves[0];
    int maxFlips = 0;

    for (const auto& move : validMoves) {
        if (isTimeUp(startTime, timeLimit)) {
            break;
        }

        Board tempBoard = board;
        if (tempBoard.makeMove(move.first, move.second, player)) {
            int currentScore = tempBoard.getScore(player);
            int originalScore = board.getScore(player);
            int flips = currentScore - originalScore - 1;
            
            if (flips > maxFlips) {
                maxFlips = flips;
                bestMove = move;
            }
        }
    }

    return bestMove;
}

REGISTER_AI_AGENT(GreedyAIAgent, "greedy")
