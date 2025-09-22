#pragma once

#include "ai_agent_base.h"
#include "bitboard.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace mcts {

class Game {
public:
  using Move = std::pair<int, int>;
  using ValidMoves = std::vector<Move>;
  explicit Game(const Board &board, CellState player)
      : next_player_(player == CellState::BLACK), winner_(-1) {
    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 8; ++col) {
        CellState cell = board.getCell(row, col);
        if (cell == CellState::BLACK) {
          bitboard_.setCell(row, col, 1);
        } else if (cell == CellState::WHITE) {
          bitboard_.setCell(row, col, 2);
        }
      }
    }
    if (bitboard_.isGameOver()) {
      compute_winner();
    }
  }
  bool next_player() const { return next_player_; }
  bool finished() const { return 0 <= winner_; }
  int winner() const { return winner_; }
  ValidMoves valid_moves() const {
    return bitboard_.getValidMoves(next_player_);
  }
  int random_play(std::minstd_rand &rng) {
    while (!finished()) {
      ValidMoves vmoves = valid_moves();
      std::uniform_int_distribution<unsigned int> dist(0u, vmoves.size() - 1);
      play(vmoves[dist(rng)]);
    }
    return winner();
  }
  void play(Move move) {
    bitboard_.makeMove(move.first, move.second, next_player_);
    bool bcp = bitboard_.hasValidMoves(1);
    bool wcp = bitboard_.hasValidMoves(0);
    if (!bcp && !wcp) {
      compute_winner();
    } else if (next_player_ && wcp) {
      next_player_ = false;
    } else if (!next_player_ && bcp) {
      next_player_ = true;
    }
  }

private:
  void compute_winner() {
    int sb = bitboard_.getScore(1);
    int sw = bitboard_.getScore(false);
    if (sb > sw) {
      winner_ = 1;
    } else if (sw > sb) {
      winner_ = 2;
    } else {
      winner_ = 0;
    }
  }
  BitBoard bitboard_;
  bool next_player_;
  int winner_;
};
class Mcts {
public:
  using time_point = std::chrono::steady_clock ::time_point;
  using time_ms = std::chrono::milliseconds;
  Mcts(unsigned int n_nodes, uint64_t seed) : rng_(seed) {
    nodes_.reserve(n_nodes);
  }
  Game::Move best_move(const Game &game, time_point start, time_ms time_limit) {
    nodes_.clear();
    Node *root = new_node(nullptr, game);
    while (nodes_.size() < nodes_.capacity() && time_left(start, time_limit)) {
      Game t_game = game;
      Node *new_node = expand(root, t_game);
      int winner = t_game.random_play(rng_);
      double score = 0;
      if (winner == 1) {
        score = 1;
      } else if (winner == 0) {
        score = 0.5;
      }
      backpropagate(new_node, score);
    }
    Game::Move best_move;
    double best_score = -std::numeric_limits<double>::infinity();
    for (unsigned int i = 0, c = root->moves.size(); i < c; ++i) {
      Node *child = root->children[i];
      if (child == nullptr) {
        continue;
      }
      double child_score = child->get_score(game.next_player());
      Game::Move move = root->moves[i];
      if (best_score < child_score) {
        best_score = child_score;
        best_move = move;
      }
    }
    return best_move;
  }

private:
  static bool time_left(time_point start, time_ms time_limit) {
    auto now = std::chrono ::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<time_ms>(now - start);
    return elapsed + time_ms{3u} < time_limit;
  }
  struct Node {
    Node *parent = nullptr;
    Game::ValidMoves moves;
    std::vector<Node *> children;
    double score = 0;
    double visits = 0;
    double get_score(bool play) const { return play ? score : visits - score; }
    double uct(Node *parent, bool play) const {
      return get_score(play) / visits + 0.2 * std::log(parent->visits / visits);
    }
  };
  Node *new_node(Node *parent, const Game &game) {
    nodes_.emplace_back();
    Node &node = nodes_.back();
    node.parent = parent;
    node.moves = game.valid_moves();
    node.children.resize(node.moves.size(), nullptr);
    return &node;
  }
  Node *expand(Node *parent, Game &game) {
    while (!game.finished()) {
      unsigned int bmi = 0;
      double b_uct = -std::numeric_limits<double>::infinity();
      unsigned int c = parent->moves.size();
      for (unsigned int i = 0; i < c; ++i) {
        Node *child = parent->children[i];
        if (child == nullptr) {
          game.play(parent->moves[i]);
          child = new_node(parent, game);
          parent->children[i] = child;
          return child;
        }
        double c_uct = child->uct(parent, game.next_player());
        if (b_uct < c_uct) {
          bmi = i;
          b_uct = c_uct;
        }
      }
      game.play(parent->moves[bmi]);
      parent = parent->children[bmi];
    }
    return parent;
  }
  void backpropagate(Node *child, double score) {
    while (child != nullptr) {
      child->score += score;
      ++child->visits;
      child = child->parent;
    }
  }
  std::minstd_rand rng_;
  std::vector<Node> nodes_;
};

} // namespace mcts

class MCTSAiAgent : public AIAgentBase {
public:
  MCTSAiAgent(const std::string &name = "MCTS",
              const std::string &author = "System");
  std::pair<int, int> getBestMove(const Board &board, CellState player,
                                  std::chrono::milliseconds timeLimit =
                                      std::chrono::milliseconds(1000)) override;

private:
  mcts::Mcts mcts_;
};
