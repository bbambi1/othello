# Othello Tournament System

A comprehensive Othello/Reversi tournament platform in modern C++17 with multiple AI agents, safety monitoring, and various tournament formats.

## Project Overview

This tournament system provides:
- **Complete Othello game engine** with proper rules and validation
- **Multiple AI agents** with different strategies and difficulty levels
- **Tournament management** with Round Robin, Single Elimination, and Swiss System formats
- **Safety monitoring** to prevent AI agents from crashing or exceeding time limits
- **Modern GUI interface** for both gameplay and tournament management
- **Comprehensive logging** and statistical analysis

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

# Build both GUI and tournament systems
make

# Run the GUI game
./othello_gui

# Run the tournament system (console-based)
./othello_tournament

# Or build/run individually
make build-tournament
make run-tournament
```

## Architecture

### Core Components
- **`Board`**: Complete Othello game engine with rules, validation, and scoring
- **`AIAgentBase`**: Abstract base class for all AI agents
- **`SimpleTournament`**: New streamlined tournament system with safety enforcement
- **`TournamentConsole`**: Console-based interface for tournament management
- **`GUIInterface`**: SFML-based graphical interface with AI selection

### AI Agent Framework
The tournament system includes multiple pre-built AI agents:
- **Random**: Makes random moves from valid options
- **Greedy**: Always picks the move that flips the most discs
- **MinMax**: Uses alpha-beta pruning with strategic evaluation

## Tournament Features

### Tournament Formats
- **Round Robin**: Every agent plays every other agent multiple times
- **Single Elimination**: Knockout tournament format
- **Swiss System**: Pairing system for balanced competition

### Safety & Monitoring
- **Time Limit Enforcement**: Prevents AI agents from taking too long
- **Crash Protection**: Monitors and handles AI agent crashes gracefully
- **Move Validation**: Ensures all moves are legal according to Othello rules
- **Resource Monitoring**: Tracks AI agent behavior and violations

### Othello Rules Implementation
- 8×8 board with proper disc flipping
- Move validation and legal move detection
- Score calculation and game state management
- Automatic turn passing when no valid moves
- Complete rule set as per [official Othello rules](https://www.ffothello.org/othello/regles-du-jeu/)

### User Interface
- **Modern GUI**: Visual gameplay with move highlighting and AI agent selection
- **Tournament Interface**: Console-based tournament management and configuration
- **Responsive Design**: Clean, intuitive interface for all game modes

## How to Use

### Playing Individual Games
1. Run `./othello_gui`
2. Choose game mode:
   - **Human vs Human**: Two human players
   - **Human vs AI**: Human plays against selected AI
   - **AI vs AI**: Two AI agents play each other
3. For AI modes, select your preferred AI agent:
   - **Random AI**: Makes random moves
   - **Greedy AI**: Always chooses the move that flips the most discs
   - **MinMax AI**: Uses strategic evaluation with alpha-beta pruning
4. Click on valid board positions to make moves
5. The game automatically highlights valid moves and shows current scores

### Running Tournaments
1. Run `./othello_tournament` to start the console interface
2. Configure tournament parameters:
   - **Select AI agents**: Choose from Random, Greedy, and MinMax agents
   - **Set time limits**: Configure how long each agent has per move (default: 5 seconds)
   - **Set rounds**: Choose how many games each pair plays (default: 1)
   - **Configure logging**: Enable/disable game logging and set log file
3. Execute the Round Robin tournament
4. View comprehensive results including:
   - Win/loss/draw records for each agent
   - Average scores and game statistics
   - Timeout and crash tracking
   - Head-to-head records between agents
5. Save results to file for analysis

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

## Tournament Features

### Safety Rules
- **Time Limit Enforcement**: Agents must complete moves within the specified time limit
- **Crash Protection**: Agents that crash or throw exceptions automatically lose the game
- **Move Validation**: All moves are validated against Othello rules
- **Resource Isolation**: Agents cannot access external resources or modify game state

### Statistics Tracking
The tournament system provides comprehensive statistics including:
- **Win/Loss/Draw Records**: Complete performance history for each agent
- **Average Scores**: Mean score achieved by each agent
- **Timeout Tracking**: Number of games lost due to time limit violations
- **Crash Tracking**: Number of games lost due to agent crashes
- **Head-to-Head Records**: Detailed matchup statistics between specific agents
- **Game Duration**: Time taken for each game
- **Move Count**: Number of moves in each game

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

# Build both GUI and tournament
make

# Build only GUI (requires SFML)
make build

# Build only tournament (no SFML required)
make build-tournament

# Run tournament
make run-tournament

# Debug build
make debug

# Check SFML availability
make check-sfml
```

## Project Structure

```
othello/
├── include/                    # Header files
│   ├── board.h                # Game board and rules
│   ├── ai_agent_base.h        # AI agent interface
│   ├── example_ai_agents.h    # AI agent implementations
│   ├── gui_interface.h        # GUI interface
│   ├── simple_tournament.h    # Tournament system
│   └── tournament_console.h   # Tournament console interface
├── src/                        # Source files
│   ├── board.cpp              # Game logic implementation
│   ├── ai_agent_base.cpp      # AI agent framework
│   ├── example_ai_agents.cpp  # AI agent implementations
│   ├── gui_interface.cpp      # GUI implementation
│   ├── main_gui.cpp           # GUI entry point
│   ├── simple_tournament.cpp  # Tournament system implementation
│   ├── tournament_console.cpp # Tournament console implementation
│   └── main_tournament_console.cpp # Tournament entry point
├── Makefile                   # Build configuration
├── othello_gui               # GUI executable
├── othello_tournament        # Tournament executable
└── README.md                 # This file
```

## Contributing

To add new AI agents:
1. Extend the `AIAgentBase` class
2. Implement the `getBestMove` method
3. Register your agent using `REGISTER_AI_AGENT`
4. Update the GUI to include your agent in the selection

## Game Rules

This implementation follows the official Othello rules. For complete rule details, visit: [Official Othello Rules](https://www.ffothello.org/othello/regles-du-jeu/)

## License

This project is open source. Feel free to use, modify, and distribute according to your needs.
