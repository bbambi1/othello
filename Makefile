CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = othello
TARGET_GUI = othello_gui
TARGET_TOURNAMENT = tournament_runner
SRCDIR = src
INCDIR = include

# Source files for different targets
SOURCES_CONSOLE = src/board.cpp src/ai_agent.cpp src/console_interface.cpp src/main.cpp
SOURCES_GUI = src/board.cpp src/ai_agent.cpp src/gui_interface.cpp src/main_gui.cpp
SOURCES_TOURNAMENT = src/board.cpp src/ai_agent.cpp src/ai_agent_base.cpp src/example_ai_agents.cpp src/safe_ai_agent.cpp src/tournament_manager.cpp src/tournament_runner.cpp examples/malicious_agent.cpp

# Object files
OBJECTS_CONSOLE = $(SOURCES_CONSOLE:.cpp=.o)
OBJECTS_GUI = $(SOURCES_GUI:.cpp=.o)
OBJECTS_TOURNAMENT = $(SOURCES_TOURNAMENT:.cpp=.o)

# SFML configuration
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system
SFML_AVAILABLE = $(shell pkg-config --exists sfml-all && echo "yes" || echo "no")

# Default target
all: $(TARGET)

# Console version (default)
$(TARGET): $(OBJECTS_CONSOLE)
	$(CXX) $(OBJECTS_CONSOLE) -o $(TARGET)

# GUI version (requires SFML)
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

# Tournament runner
$(TARGET_TOURNAMENT): $(OBJECTS_TOURNAMENT)
	$(CXX) $(OBJECTS_TOURNAMENT) -o $(TARGET_TOURNAMENT)

# Compile source files with different flags for GUI
src/gui_interface.o: src/gui_interface.cpp
	$(CXX) $(CXXFLAGS) -DUSE_SFML -I$(INCDIR) -c $< -o $@

src/main_gui.o: src/main_gui.cpp
	$(CXX) $(CXXFLAGS) -DUSE_SFML -I$(INCDIR) -c $< -o $@

# Compile other source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS_CONSOLE) $(OBJECTS_GUI) $(OBJECTS_TOURNAMENT) $(TARGET) $(TARGET_GUI) $(TARGET_TOURNAMENT)

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

# Run the console game
run: $(TARGET)
	./$(TARGET)

# Run the GUI game
run-gui: $(TARGET_GUI)
	./$(TARGET_GUI)

# Run tournament
run-tournament: $(TARGET_TOURNAMENT)
	./$(TARGET_TOURNAMENT)

# Build all versions
build-all: $(TARGET) $(TARGET_GUI) $(TARGET_TOURNAMENT)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# GUI debug build
debug-gui: CXXFLAGS += -g -DDEBUG
debug-gui: $(TARGET_GUI)

# Check SFML availability
check-sfml:
	@echo "SFML availability: $(SFML_AVAILABLE)"
	@if [ "$(SFML_AVAILABLE)" = "yes" ]; then \
		echo "SFML is available. You can build the GUI version."; \
	else \
		echo "SFML is not available. Install SFML development libraries."; \
	fi

.PHONY: all clean install-deps install-sfml run run-gui run-tournament build-all debug debug-gui check-sfml
