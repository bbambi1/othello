# Othello AI Competition Framework

Welcome to the Othello AI Competition Framework! This project provides a complete environment for creating, testing, and competing AI agents in the classic game of Othello/Reversi.

## 🎯 What is This?

This is a framework for hosting AI agent competitions in Othello. Participants create AI agents that compete against each other using various tournament formats. The framework includes:

- **Game Engine**: Complete Othello rules implementation
- **AI Agent Framework**: Base classes and interfaces for creating agents
- **Tournament System**: Multiple tournament formats (Round Robin, Single Elimination, Swiss System)
- **GUI Interface**: Visual game interface using SFML
- **Console Interface**: Terminal-based interface for headless operation
- **Example Agents**: Several AI implementations to learn from

## 🚀 Quick Start

### 1. Build the Project

```bash
# Check if SFML is available
make check-sfml

# Install SFML if needed (Ubuntu/Debian)
make install-sfml

# Build all components
make build-all
```

### 2. Run a Quick Tournament

```bash
# Run a simple tournament with example agents
./tournament_runner --agents random,greedy,minmax --tournament roundrobin --rounds 2
```

### 3. Play Against AI

```bash
# Console interface
./othello

# GUI interface (if SFML is available)
./othello_gui
```

## 🤖 Creating Your Own AI Agent

### Step 1: Create Your Agent Class

Create a new file `my_ai_agent.h`:

```cpp
#include "ai_agent_base.h"

class MyAIAgent : public AIAgentBase {
public:
    MyAIAgent(const std::string& name = "MyAgent");
    
    // This is the main method you must implement!
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    
    // Optional: Override these for additional functionality
    void onGameStart() override;
    void onMoveMade(int row, int col, CellState player) override;
    void onGameEnd(CellState winner) override;
    
private:
    // Add your private methods and data here
    double evaluatePosition(const Board& board, int row, int col, CellState player) const;
};
```

### Step 2: Implement Your Agent

Create `my_ai_agent.cpp`:

```cpp
#include "my_ai_agent.h"

MyAIAgent::MyAIAgent(const std::string& name) : AIAgentBase(name) {
}

std::pair<int, int> MyAIAgent::getBestMove(const Board& board, CellState player) {
    auto validMoves = getValidMoves(board, player);
    if (validMoves.empty()) {
        return {-1, -1}; // No valid moves
    }
    
    // Your strategy goes here!
    // This is where you implement your AI logic
    
    // For now, just return the first valid move
    return validMoves[0];
}

// Register your agent
REGISTER_AI_AGENT(MyAIAgent, "myagent")
```

### Step 3: Build and Test

```bash
# Add your files to src/ and include/ directories
# Rebuild the project
make clean
make build-all

# Test your agent
./tournament_runner --agents random,myagent --tournament roundrobin
```

## 🏆 Tournament Formats

### Round Robin
- Every agent plays against every other agent
- Configurable number of rounds per matchup
- Best for determining overall ranking

```bash
./tournament_runner --agents agent1,agent2,agent3 --tournament roundrobin --rounds 3
```

### Single Elimination
- Agents are paired and losers are eliminated
- Fastest tournament format
- Good for large numbers of agents

```bash
./tournament_runner --agents agent1,agent2,agent3,agent4 --tournament singleelim
```

### Swiss System
- Agents are paired based on similar scores
- Multiple rounds with changing opponents
- Good balance between fairness and speed

```bash
./tournament_runner --agents agent1,agent2,agent3,agent4 --tournament swiss --rounds 5
```

## 🎮 Available Example Agents

### Random Agent (`random`)
- Chooses moves randomly from valid options
- Good baseline for testing
- Very fast execution

### Greedy Agent (`greedy`)
- Always chooses the move that flips the most discs
- Simple but effective early-game strategy
- Fast execution

### MinMax Agent (`minmax`)
- Uses MinMax algorithm with alpha-beta pruning
- Configurable search depth
- Strong but slower execution

### Positional Agent (`positional`)
- Focuses on controlling strategic board positions
- Avoids moves that give opponents corner access
- Good positional understanding

### Hybrid Agent (`hybrid`)
- Combines multiple evaluation strategies
- Uses MinMax with sophisticated evaluation
- Strong overall performance

## 🛠️ Development Tools

### Helper Methods Available

Your agent can use these helper methods from `AIAgentBase`:

```cpp
// Get valid moves for a player
auto validMoves = getValidMoves(board, player);

// Check if a move is valid
bool isValid = isValidMove(board, row, col, player);

// Get opponent's color
CellState opponent = getOpponent(player);

// Common evaluation functions
double cornerScore = evaluateCornerControl(board, player);
double edgeScore = evaluateEdgeControl(board, player);
double mobilityScore = evaluateMobility(board, player);
double discScore = evaluateDiscCount(board, player);
double stabilityScore = evaluateStability(board, player);
```

### Time Management

```cpp
// Check if time limit is approaching
bool timeUp = isTimeUp(startTime, timeLimit);
```

## 📊 Understanding the Game

### Key Strategic Concepts

1. **Corner Control**: Corners are extremely valuable (25 points each)
2. **Edge Control**: Edges provide stability (5 points each)
3. **Mobility**: Having more valid moves is advantageous
4. **Stability**: Avoid discs that can be easily flipped back
5. **Disc Count**: Generally, more discs is better (but not always!)

### Board Positions

```
  0 1 2 3 4 5 6 7
0 C E E E E E E C
1 E I I I I I I E
2 E I I I I I I E
3 E I I I I I I E
4 E I I I I I I E
5 E I I I I I I E
6 E I I I I I I E
7 C E E E E E E C

C = Corner (most valuable)
E = Edge (valuable)
I = Interior (less valuable)
```

## 🔧 Advanced Features

### Custom Evaluation Functions

```cpp
double MyAIAgent::evaluateBoard(const Board& board, CellState player) const {
    double score = 0.0;
    
    // Use built-in evaluation functions
    score += evaluateCornerControl(board, player) * 30.0;
    score += evaluateEdgeControl(board, player) * 8.0;
    score += evaluateMobility(board, player) * 15.0;
    
    // Add your custom evaluation
    score += evaluateMyCustomStrategy(board, player);
    
    return score;
}
```

### Opening Book

```cpp
std::pair<int, int> MyAIAgent::getBestMove(const Board& board, CellState player) {
    // Check opening book for early game
    if (board.getTotalDiscs() < 20) {
        auto openingMove = getOpeningBookMove(board, player);
        if (openingMove.first >= 0) {
            return openingMove;
        }
    }
    
    // Fall back to your main strategy
    return getMainStrategyMove(board, player);
}
```

### Endgame Database

```cpp
std::pair<int, int> MyAIAgent::getBestMove(const Board& board, CellState player) {
    // Use endgame database for late game
    if (board.getTotalDiscs() > 50) {
        auto endgameMove = getEndgameDatabaseMove(board, player);
        if (endgameMove.first >= 0) {
            return endgameMove;
        }
    }
    
    // Fall back to your main strategy
    return getMainStrategyMove(board, player);
}
```

## 📈 Performance Optimization

### Search Depth vs. Speed

- **Depth 1-3**: Very fast, suitable for rapid tournaments
- **Depth 4-6**: Good balance of strength and speed
- **Depth 7+**: Strong play but slower execution

### Time Management

```cpp
std::pair<int, int> MyAIAgent::getBestMove(const Board& board, CellState player) {
    auto startTime = std::chrono::steady_clock::now();
    auto timeLimit = std::chrono::milliseconds(5000); // 5 seconds
    
    // Iterative deepening
    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (isTimeUp(startTime, timeLimit)) {
            break; // Use best move found so far
        }
        
        // Search at current depth
        auto move = searchAtDepth(board, player, depth);
        if (move.first >= 0) {
            bestMove = move;
        }
    }
    
    return bestMove;
}
```

## 🏁 Running Competitions

### Local Tournament

```bash
# Create a tournament with your agents
./tournament_runner \
    --agents myagent1,myagent2,myagent3,minmax,hybrid \
    --tournament roundrobin \
    --rounds 3 \
    --output my_tournament_results.txt
```

### Headless Mode

```bash
# Run tournaments without GUI (good for servers)
./tournament_runner \
    --agents agent1,agent2,agent3 \
    --tournament swiss \
    --rounds 5 \
    --output results.txt \
    --log tournament.log
```

### Analyzing Results

```bash
# View tournament results
cat tournament_results.txt

# View detailed game logs
cat game_logs.txt

# View tournament log
cat tournament.log
```

## 🐛 Debugging Your Agent

### Enable Debug Output

```cpp
class MyAIAgent : public AIAgentBase {
public:
    MyAIAgent(const std::string& name) : AIAgentBase(name) {
        debugMode = true; // Enable debug output
    }
    
    std::pair<int, int> getBestMove(const Board& board, CellState player) override {
        if (debugMode) {
            std::cout << "MyAgent: Evaluating board for " 
                      << (player == CellState::BLACK ? "Black" : "White") << std::endl;
        }
        
        // Your logic here
    }
    
private:
    bool debugMode = false;
};
```

### Test Individual Games

```bash
# Test your agent against a known agent
./tournament_runner --agents myagent,random --tournament roundrobin --rounds 1
```

## 📚 Learning Resources

### Othello Strategy
- [Othello Strategy Guide](https://en.wikipedia.org/wiki/Reversi#Strategy)
- [Othello Opening Theory](https://www.othellobook.com/)
- [Endgame Analysis](https://www.othellobook.com/endgame)

### AI Algorithms
- [MinMax Algorithm](https://en.wikipedia.org/wiki/Minimax)
- [Alpha-Beta Pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning)
- [Game Tree Search](https://en.wikipedia.org/wiki/Game_tree)

### C++ Resources
- [Modern C++](https://isocpp.github.io/CppCoreGuidelines/)
- [SFML Documentation](https://www.sfml-dev.org/documentation.php)

## 🤝 Contributing

### Adding New Features

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Add tests if applicable
5. Submit a pull request

### Reporting Issues

- Use the GitHub issue tracker
- Include system information
- Provide steps to reproduce
- Attach relevant logs

### Suggesting Improvements

- Open a discussion on GitHub
- Describe the proposed feature
- Explain the benefits
- Provide implementation ideas if possible

## 📄 License

This project is open source and available under the MIT License. See the LICENSE file for details.

## 🙏 Acknowledgments

- Classic Othello/Reversi game rules
- SFML development team for the graphics library
- All contributors to the AI competition framework

---

**Good luck with your AI agents! May the best algorithm win! 🏆**
