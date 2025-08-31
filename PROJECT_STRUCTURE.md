# Othello AI Competition Framework - Project Structure

This document provides an overview of the project structure and explains how different components work together.

## Directory Structure

```
othello/
├── include/                     # Header files
│   ├── board.h                 # Game board logic and rules
│   ├── ai_agent.h              # Original AI agent (MinMax)
│   ├── console_interface.h     # Terminal-based interface
│   ├── gui_interface.h         # SFML-based graphical interface
│   ├── ai_agent_base.h         # Base class for competition agents
│   ├── example_ai_agents.h     # Example AI implementations
│   └── tournament_manager.h    # Tournament system
├── src/                        # Source files
│   ├── board.cpp               # Board implementation
│   ├── ai_agent.cpp            # Original AI implementation
│   ├── console_interface.cpp   # Console interface
│   ├── gui_interface.cpp       # GUI interface
│   ├── ai_agent_base.cpp       # Base agent implementation
│   ├── example_ai_agents.cpp   # Example agents
│   ├── tournament_manager.cpp  # Tournament system
│   ├── tournament_runner.cpp   # Tournament runner program
│   ├── main.cpp                # Console main program
│   └── main_gui.cpp            # GUI main program
├── examples/                   # Example AI agents for participants
│   ├── my_first_ai_agent.h    # Example agent header
│   └── my_first_ai_agent.cpp  # Example agent implementation
├── build/                      # Build artifacts (generated)
├── Makefile                    # Build configuration
├── CMakeLists.txt              # CMake configuration
├── README.md                   # Main project documentation
├── COMPETITION_README.md       # Competition-specific documentation
├── PROJECT_STRUCTURE.md        # This file
└── build_my_agent.sh          # Helper script for participants
```

## Core Components

### 1. Game Engine (`board.h/cpp`)
- **Purpose**: Implements the complete Othello game rules
- **Key Features**:
  - Move validation
  - Disc flipping logic
  - Game state management
  - Score calculation
- **Usage**: All other components depend on this

### 2. AI Agent Framework (`ai_agent_base.h/cpp`)
- **Purpose**: Provides base classes and interfaces for AI agents
- **Key Features**:
  - Abstract base class `AIAgentBase`
  - Common evaluation functions
  - Helper methods for move generation
  - Agent registration system
- **Usage**: Participants extend this class to create their agents

### 3. Tournament System (`tournament_manager.h/cpp`)
- **Purpose**: Manages competitions between AI agents
- **Key Features**:
  - Multiple tournament formats (Round Robin, Single Elimination, Swiss)
  - Game result tracking
  - Statistics and rankings
  - Detailed logging
- **Usage**: Run competitions and analyze results

### 4. User Interfaces
- **Console Interface** (`console_interface.h/cpp`): Terminal-based gameplay
- **GUI Interface** (`gui_interface.h/cpp`): SFML-based graphical interface

## How to Use

### For Participants (AI Agent Developers)

1. **Study the Framework**:
   - Read `COMPETITION_README.md`
   - Examine `examples/my_first_ai_agent.cpp`
   - Understand `ai_agent_base.h`

2. **Create Your Agent**:
   - Extend `AIAgentBase` class
   - Implement `getBestMove()` method
   - Register your agent with `REGISTER_AI_AGENT`

3. **Test Your Agent**:
   - Use `build_my_agent.sh` script
   - Run tournaments with `./tournament_runner`
   - Play against your agent

### For Tournament Organizers

1. **Setup**:
   - Build the project: `make build-all`
   - Check SFML availability: `make check-sfml`

2. **Run Tournaments**:
   - Use tournament runner: `./tournament_runner --help`
   - Configure tournament parameters
   - Analyze results

## Component Dependencies

```
ai_agent_base.h ← example_ai_agents.h
       ↓
tournament_manager.h
       ↓
tournament_runner.cpp

board.h ← ai_agent_base.h
   ↓
console_interface.h
gui_interface.h
```

## Key Classes and Their Roles

### `AIAgentBase`
- **Purpose**: Base class for all AI agents
- **Key Methods**:
  - `getBestMove()`: Main method to implement
  - `onGameStart()`, `onMoveMade()`, `onGameEnd()`: Optional hooks
- **Helper Methods**: Evaluation functions, move validation, etc.

### `TournamentManager`
- **Purpose**: Manages tournament execution
- **Key Methods**:
  - `runRoundRobin()`, `runSingleElimination()`, `runSwissSystem()`
  - `playGame()`: Executes individual games
  - `getResults()`: Retrieves tournament statistics

### `Board`
- **Purpose**: Game state and rules
- **Key Methods**:
  - `isValidMove()`, `makeMove()`, `getValidMoves()`
  - `getScore()`, `isGameOver()`, `getFlippedDiscs()`

## Extension Points

### Adding New AI Agents
1. Create new class extending `AIAgentBase`
2. Implement `getBestMove()` method
3. Register with `REGISTER_AI_AGENT` macro
4. Rebuild project

### Adding New Tournament Formats
1. Extend `TournamentManager` class
2. Implement new tournament algorithm
3. Add public method for the new format
4. Update tournament runner

### Adding New Evaluation Functions
1. Add method to `AIAgentBase`
2. Implement evaluation logic
3. Make available to derived classes

## Debugging and Development

### Common Issues
- **Build Errors**: Check SFML installation, C++17 support
- **Runtime Errors**: Verify agent registration, move validation
- **Performance Issues**: Check search depth, evaluation complexity

### Development Tips
- Use `make debug` for debug builds
- Enable logging in tournament manager
- Test agents individually before tournaments
- Use existing agents as benchmarks

## Learning Path

1. **Start Simple**: Understand the basic game rules
2. **Study Examples**: Analyze existing AI agents
3. **Create Basic Agent**: Implement simple strategies
4. **Optimize**: Improve evaluation functions and search
5. **Compete**: Enter tournaments and analyze results

## Build Process

```bash
# Check dependencies
make check-sfml

# Build console version
make

# Build GUI version
make othello_gui

# Build tournament runner
make tournament_runner

# Build everything
make build-all

# Clean build artifacts
make clean
```

## Next Steps

After understanding this structure:

1. **Read the Competition README**: `COMPETITION_README.md`
2. **Try the Example Agent**: Copy from `examples/` directory
3. **Build and Test**: Use `build_my_agent.sh` script
4. **Create Your Strategy**: Implement your own AI logic
5. **Compete**: Enter tournaments and see how you rank!

---

**Happy coding and good luck in the competition!**
