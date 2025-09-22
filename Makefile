CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET_GUI = othello_gui
TARGET_TOURNAMENT = othello_tournament
TARGET_CONSOLE = othello_console
SRCDIR = src
INCDIR = include

# Source files for GUI target
SOURCES_GUI = src/board.cpp src/bitboard.cpp src/ai_agent_base.cpp agents/src/random_ai_agent.cpp agents/src/greedy_ai_agent.cpp agents/src/minmax_ai_agent.cpp agents/src/bitboard_ai_agent.cpp src/gui_interface.cpp src/main_gui.cpp

# Source files for Tournament target
SOURCES_TOURNAMENT = src/board.cpp src/bitboard.cpp src/ai_agent_base.cpp agents/src/random_ai_agent.cpp agents/src/greedy_ai_agent.cpp agents/src/minmax_ai_agent.cpp agents/src/bitboard_ai_agent.cpp src/simple_tournament.cpp src/tournament_console.cpp src/main_tournament_console.cpp

# Source files for Console target
SOURCES_CONSOLE = src/board.cpp src/bitboard.cpp src/ai_agent_base.cpp agents/src/random_ai_agent.cpp agents/src/greedy_ai_agent.cpp agents/src/minmax_ai_agent.cpp agents/src/bitboard_ai_agent.cpp src/console_game.cpp src/main_console.cpp

# Object files
OBJECTS_GUI = $(SOURCES_GUI:.cpp=.o)
OBJECTS_TOURNAMENT = $(SOURCES_TOURNAMENT:.cpp=.o)
OBJECTS_CONSOLE = $(SOURCES_CONSOLE:.cpp=.o)

# SFML configuration
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system
SFML_AVAILABLE = $(shell pkg-config --exists sfml-all && echo "yes" || echo "no")

# Default target
all: $(TARGET_GUI) $(TARGET_TOURNAMENT) $(TARGET_CONSOLE)

# GUI version
ifneq ($(SFML_AVAILABLE),no)
$(TARGET_GUI): $(OBJECTS_GUI)
	$(CXX) $(OBJECTS_GUI) $(SFML_LIBS) -o $(TARGET_GUI)
else
$(TARGET_GUI):
	@echo "SFML not found. Install SFML development libraries first."
	@echo "Ubuntu/Debian: sudo apt-get install libsfml-dev"
	@echo "Fedora: sudo dnf install SFML-devel"
	@echo "macOS: brew install sfml"
	@echo "Windows: Download from https://www.sfml-dev.org/download.php"
	@exit 1
endif

# Tournament version (console-based, no SFML required)
$(TARGET_TOURNAMENT): $(OBJECTS_TOURNAMENT)
	$(CXX) $(OBJECTS_TOURNAMENT) -o $(TARGET_TOURNAMENT)

# Console game version (no SFML required)
$(TARGET_CONSOLE): $(OBJECTS_CONSOLE)
	$(CXX) $(OBJECTS_CONSOLE) -o $(TARGET_CONSOLE)

# Compile source files with different flags for GUI
src/gui_interface.o: src/gui_interface.cpp
	$(CXX) $(CXXFLAGS) -DUSE_SFML -I$(INCDIR) -Iagents/include -c $< -o $@

src/main_gui.o: src/main_gui.cpp
	$(CXX) $(CXXFLAGS) -DUSE_SFML -I$(INCDIR) -Iagents/include -c $< -o $@

# Compile other source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -Iagents/include -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS_GUI) $(OBJECTS_TOURNAMENT) $(OBJECTS_CONSOLE) $(TARGET_GUI) $(TARGET_TOURNAMENT) $(TARGET_CONSOLE)

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y g++ make
	@echo "Installing SFML development libraries..."
	sudo apt-get install -y libsfml-dev

# Install SFML (Ubuntu/Debian)
install-sfml:
	sudo apt-get update
	sudo apt-get install -y libsfml-dev

# Run the GUI game
run-gui: $(TARGET_GUI)
	./$(TARGET_GUI)

# Run the tournament
run-tournament: $(TARGET_TOURNAMENT)
	./$(TARGET_TOURNAMENT)

# Run the console game
run-console: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE)

# Build GUI version
build-gui: $(TARGET_GUI)

# Build tournament version
build-tournament: $(TARGET_TOURNAMENT)

# Build console version
build-console: $(TARGET_CONSOLE)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET_GUI) $(TARGET_TOURNAMENT) $(TARGET_CONSOLE)

# Check SFML availability
check-sfml:
	@echo "SFML availability: $(SFML_AVAILABLE)"
	@if [ "$(SFML_AVAILABLE)" = "yes" ]; then \
		echo "SFML is available. You can build the GUI version."; \
	else \
		echo "SFML is not available. Install SFML development libraries."; \
	fi

.PHONY: all clean install-deps install-sfml run run-tournament run-console build build-tournament build-console debug check-sfml
