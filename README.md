# Othello AI Competition Framework

A complete, production-ready framework for hosting AI agent competitions in the classic game of Othello/Reversi. Built in modern C++17 with SFML graphics support.

## Project Overview

This framework provides everything needed to:
- **Create AI agents** that compete in Othello
- **Run tournaments** with multiple formats and scoring
- **Visualize games** through both console and GUI interfaces
- **Analyze results** with comprehensive statistics and logging

## Quick Start

### Prerequisites
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- SFML development libraries (optional, for GUI)
- Make utility

### Build & Run
```bash
# Check SFML availability
make check-sfml

# Install SFML if needed (Ubuntu/Debian)
make install-sfml

# Build all components
make build-all

# Run console version
./othello

# Run GUI version
./othello_gui

# Run tournament
./tournament_runner --agents random,minmax --tournament roundrobin --rounds 3
```

## Architecture

### Core Components
- **`Board`**: Complete Othello game engine with rules, validation, and scoring
- **`AIAgentBase`**: Abstract base class for all AI agents
- **`TournamentManager`**: Tournament system supporting multiple formats
- **`ConsoleInterface`**: Terminal-based gameplay interface
- **`GUIInterface`**: SFML-based graphical interface

### AI Agent Framework
Participants extend `AIAgentBase` and implement:
```cpp
class MyAgent : public AIAgentBase {
public:
    std::pair<int, int> getBestMove(const Board& board, CellState player) override;
    // Optional: onGameStart(), onMoveMade(), onGameEnd()
};

REGISTER_AI_AGENT(MyAgent, "myagent")
```

### Tournament Formats
- **Round Robin**: Every agent plays every other agent
- **Single Elimination**: Knockout tournament
- **Swiss System**: Pairing based on similar scores

## Game Features

### Othello Rules Implementation
- 8×8 board with proper disc flipping
- Move validation and legal move detection
- Score calculation and game state management
- Automatic turn passing when no valid moves

### AI Agent Examples
- **Random**: Chooses moves randomly
- **Greedy**: Always picks move that flips most discs
- **MinMax**: Alpha-beta pruning with configurable depth
- **Positional**: Focuses on strategic board positions
- **Hybrid**: Combines multiple evaluation strategies

### User Interfaces
- **Console**: Fast, headless operation for tournaments
- **GUI**: Visual gameplay with move highlighting and animations
- **Tournament Runner**: Command-line tool for competitions

## Tournament System

### Features
- **Multiple Formats**: Round Robin, Single Elimination, Swiss
- **Statistics Tracking**: Win rates, scores, game counts
- **Detailed Logging**: Game logs, tournament results, performance metrics
- **Fair Play**: Time limits, move validation, consistent rules

### Usage Examples
```bash
# Quick test tournament
./tournament_runner --agents random,greedy,minmax --tournament roundrobin

# Swiss system tournament
./tournament_runner --agents random,greedy,minmax,positional,hybrid --tournament swiss --rounds 5

# Single elimination
./tournament_runner --agents agent1,agent2,agent3,agent4 --tournament singleelim
```

## Development

### Creating Your AI Agent
1. **Study Examples**: Review `examples/my_first_ai_agent.cpp`
2. **Extend Base Class**: Inherit from `AIAgentBase`
3. **Implement Strategy**: Override `getBestMove()` method
4. **Register Agent**: Use `REGISTER_AI_AGENT` macro
5. **Test & Compete**: Use tournament runner to evaluate performance

### Available Helper Methods
```cpp
// Move generation and validation
auto validMoves = getValidMoves(board, player);
bool isValid = isValidMove(board, row, col, player);

// Evaluation functions
double cornerScore = evaluateCornerControl(board, player);
double edgeScore = evaluateEdgeControl(board, player);
double mobilityScore = evaluateMobility(board, player);

// Game state
CellState opponent = getOpponent(player);
bool timeUp = isTimeUp(startTime, timeLimit);
```

### Build System
- **Makefile**: Separate targets for console, GUI, and tournament
- **SFML Integration**: Automatic detection and conditional compilation
- **Debug Support**: Debug builds with additional flags

## Project Structure

```
othello/
├── include/                     # Header files
│   ├── board.h                 # Game engine
│   ├── ai_agent_base.h         # AI framework
│   ├── tournament_manager.h    # Tournament system
│   ├── gui_interface.h         # SFML interface
│   └── console_interface.h     # Terminal interface
├── src/                        # Implementation files
├── examples/                   # Example AI agents
├── Makefile                    # Build configuration
├── COMPETITION_README.md       # Detailed competition guide
└── PROJECT_STRUCTURE.md        # Technical architecture
```

## Configuration

### Build Options
```bash
make                    # Console version only
make othello_gui        # GUI version (requires SFML)
make tournament_runner  # Tournament system
make build-all          # All components
make debug              # Debug build
make clean              # Clean build artifacts
```

### Tournament Settings
- **Rounds per matchup**: Configurable for round robin
- **Time limits**: Per-move time constraints
- **Logging**: Game logs and tournament results
- **Output files**: Customizable result storage

## Performance

### AI Agent Capabilities
- **Random**: ~1ms per move
- **Greedy**: ~1ms per move
- **MinMax (depth 4)**: ~100ms per move
- **MinMax (depth 6)**: ~1s per move
- **MinMax (depth 8+)**: 1-10s per move

### Tournament Performance
- **Round Robin**: O(n²) complexity for n agents
- **Single Elimination**: O(n log n) complexity
- **Swiss System**: O(n × rounds) complexity

## Debugging & Testing

### Debug Tools
- **Individual Game Testing**: Use debug test program
- **Tournament Logs**: Detailed game-by-game analysis
- **Performance Profiling**: Move timing and statistics
- **Error Handling**: Comprehensive error messages and validation

### Common Issues
- **Build Errors**: Check SFML installation and C++17 support
- **Runtime Errors**: Verify agent registration and move validation
- **Performance Issues**: Adjust search depth and evaluation complexity

## Documentation

### Guides
- **`COMPETITION_README.md`**: Complete competition framework guide
- **`PROJECT_STRUCTURE.md`**: Technical architecture and dependencies
- **`examples/`**: Working AI agent implementations
- **`build_my_agent.sh`**: Helper script for participants

### Learning Resources
- Othello strategy guides and opening theory
- AI algorithms (MinMax, Alpha-Beta pruning)
- Modern C++ programming practices
- SFML graphics library documentation

## Contributing

### Adding Features
1. **AI Agents**: Extend base class with new strategies
2. **Tournament Formats**: Implement new competition algorithms
3. **Evaluation Functions**: Add new board assessment methods
4. **User Interfaces**: Enhance console or GUI functionality

### Code Quality
- Modern C++17 practices
- Comprehensive error handling
- Performance optimization
- Cross-platform compatibility

## License

MIT License - Open source and free for educational and commercial use.

## Competition Ready

This framework is designed for:
- **Educational Use**: Learning AI algorithms and game theory
- **Research**: Testing new AI strategies and evaluation functions
- **Competitions**: Hosting fair and engaging AI tournaments
- **Development**: Building and testing AI agents

## Getting Started

1. **Build the project**: `make build-all`
2. **Try the examples**: Run tournaments with included agents
3. **Create your agent**: Extend `AIAgentBase` class
4. **Test your strategy**: Use tournament runner for evaluation
5. **Compete**: Enter tournaments and analyze results

---

**Ready to create the next champion AI agent? Start coding!**
