# Othello tournament

The tournament uses the [french rules](https://www.ffothello.org/othello/regles-du-jeu/).

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

## How to Use

### Playing Individual Games
1. Run `./othello_gui`
2. Choose game mode:
   - **Human vs Human**: Two human players
   - **Human vs AI**: Human plays against selected AI
   - **AI vs AI**: Two AI agents play each other
3. Click on valid board positions to make moves
4. The game automatically highlights valid moves and shows current scores

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

## Contributing

### Adding New AI Agents

To create a new AI agent and integrate it into both the tournament and GUI:

1. **Create agent files** in the `agents/` folder:
   - `agents/include/your_ai_agent.h` - Header file
   - `agents/src/your_ai_agent.cpp` - Implementation file

2. **Implement your agent**:
   ```cpp
   class YourAIAgent : public AIAgentBase {
   public:
       YourAIAgent(const std::string& name = "YourAI", const std::string& author = "Your Name");
       std::pair<int, int> getBestMove(const Board& board, CellState player, 
                                      std::chrono::milliseconds timeLimit) override;
   };
   
   // Register the agent
   REGISTER_AI_AGENT(YourAIAgent, "your_type")
   ```

3. **Update build system**:
   - Add `agents/src/your_ai_agent.cpp` to both `SOURCES_GUI` and `SOURCES_TOURNAMENT` in `Makefile`

4. **Update tournament system**:
   - Add `"your_type"` to `availableAgentTypes_` in `src/tournament_console.cpp`

5. **Update GUI interface**:
   - Add `YOUR_TYPE` to `AIAgentType` enum in `include/gui_interface.h`
   - Add agent option, key handling, and type conversion in `src/gui_interface.cpp`

6. **Build and test**:
   ```bash
   make clean && make
   ```

Your agent will be available in both the tournament system and GUI automatically!

## License

This project is open source. Feel free to use, modify, and distribute according to your needs.
