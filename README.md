# Othello Game with AI Agents

A complete Othello/Reversi game implementation in modern C++17 with SFML graphics support and three different AI agents.

## Project Overview

This project provides:
- **Complete Othello game engine** with proper rules and validation
- **Three AI agents** with different strategies
- **Modern GUI interface** with AI agent selection
- **Easy-to-use interface** for both human and AI gameplay

## Quick Start

### Prerequisites
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- SFML development libraries (required for GUI)
- Make utility

### Build & Run
```bash
# Check SFML availability
make check-sfml

# Install SFML if needed (Ubuntu/Debian)
make install-sfml

# Build the game
make

# Run the game
./othello_gui
```

## Architecture

### Core Components
- **`Board`**: Complete Othello game engine with rules, validation, and scoring
- **`AIAgentBase`**: Abstract base class for all AI agents
- **`GUIInterface`**: SFML-based graphical interface with AI selection

### AI Agent Framework
The project includes three pre-built AI agents:
- **Random**: Makes random moves from valid options
- **Greedy**: Always picks the move that flips the most discs
- **MinMax**: Uses alpha-beta pruning with strategic evaluation

## Game Features

### Othello Rules Implementation
- 8×8 board with proper disc flipping
- Move validation and legal move detection
- Score calculation and game state management
- Automatic turn passing when no valid moves

### User Interface
- **Modern GUI**: Visual gameplay with move highlighting and AI agent selection
- **Responsive Design**: Clean, intuitive interface for all game modes

## How to Play

### Getting Started
1. Run `./othello_gui`
2. Choose game mode:
   - **Human vs Human**: Two human players
   - **Human vs AI**: Human plays against selected AI
   - **AI vs AI**: Two AI agents play each other
3. For AI modes, select your preferred AI agent:
   - **Random AI**: Makes random moves
   - **Greedy AI**: Always chooses the move that flips the most discs
   - **MinMax AI**: Uses strategic evaluation with alpha-beta pruning
4. For AI vs AI mode, you'll also select the opponent AI agent
5. Click on valid board positions to make moves
6. The game automatically highlights valid moves and shows current scores

## AI Agent Selection

### Human vs AI Mode
- Choose which AI agent you want to play against
- The AI will play as White (second player)
- You'll see which AI agent is being used in the UI

### AI vs AI Mode
- Select the AI agent for Black (first player)
- Select the AI agent for White (second player)
- Watch two different AI strategies compete
- Perfect for testing and comparing AI performance

### Available AI Agents

## AI Agent Details

### Random AI
- **Strategy**: Random selection from valid moves
- **Use case**: Baseline opponent, testing game mechanics
- **Difficulty**: Very easy

### Greedy AI
- **Strategy**: Always chooses the move that captures the most opponent discs
- **Use case**: Intermediate opponent, good for learning basic strategy
- **Difficulty**: Easy to medium

### MinMax AI
- **Strategy**: Alpha-beta pruning with configurable search depth
- **Evaluation**: Considers corner control, edge control, mobility, and disc count
- **Use case**: Challenging opponent, demonstrates advanced AI techniques
- **Difficulty**: Medium to hard

## Building from Source

### Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install g++ make libsfml-dev

# Fedora
sudo dnf install gcc-c++ make SFML-devel

# macOS
brew install gcc make sfml
```

### Build Commands
```bash
# Clean build
make clean

# Build the game
make

# Debug build
make debug

# Check SFML availability
make check-sfml
```

## Project Structure

```
othello/
├── include/           # Header files
│   ├── board.h       # Game board and rules
│   ├── ai_agent_base.h  # AI agent interface
│   ├── example_ai_agents.h  # AI agent implementations
│   └── gui_interface.h     # GUI interface
├── src/               # Source files
│   ├── board.cpp     # Game logic implementation
│   ├── ai_agent_base.cpp  # AI agent framework
│   ├── example_ai_agents.cpp  # AI agent implementations
│   ├── gui_interface.cpp     # GUI implementation
│   └── main_gui.cpp  # GUI entry point
├── Makefile          # Build configuration
└── README.md         # This file
```

## Contributing

To add new AI agents:
1. Extend the `AIAgentBase` class
2. Implement the `getBestMove` method
3. Register your agent using `REGISTER_AI_AGENT`
4. Update the GUI to include your agent in the selection

## License

This project is open source. Feel free to use, modify, and distribute according to your needs.
