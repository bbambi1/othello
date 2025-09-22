#pragma GCC optimize("-Ofast,inline,omit-frame-pointer,unroll-loops,fast-math")
#include "mcts_ai_agent.h"

MCTSAiAgent::MCTSAiAgent(const std::string &name, const std::string &author)
    : AIAgentBase(name, author), mcts_(1'000'000, 14785) {}

std::pair<int, int>
MCTSAiAgent::getBestMove(const Board &board, CellState player,
                         std::chrono::milliseconds timeLimit) {
  mcts::Game game(board, player);
  return mcts_.best_move(game, std::chrono::steady_clock::now(), timeLimit);
}

REGISTER_AI_AGENT(MCTSAiAgent, "mcts");
