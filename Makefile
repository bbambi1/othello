CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET_GUI = othello_gui
SRCDIR = src
INCDIR = include

# Source files for GUI target
SOURCES_GUI = src/board.cpp src/ai_agent_base.cpp src/example_ai_agents.cpp src/gui_interface.cpp src/main_gui.cpp

# Object files
OBJECTS_GUI = $(SOURCES_GUI:.cpp=.o)

# SFML configuration
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system
SFML_AVAILABLE = $(shell pkg-config --exists sfml-all && echo "yes" || echo "no")

# Default target
all: $(TARGET_GUI)

# GUI version (default)
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
	rm -f $(OBJECTS_GUI) $(TARGET_GUI)

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
run: $(TARGET_GUI)
	./$(TARGET_GUI)

# Build GUI version
build: $(TARGET_GUI)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET_GUI)

# Check SFML availability
check-sfml:
	@echo "SFML availability: $(SFML_AVAILABLE)"
	@if [ "$(SFML_AVAILABLE)" = "yes" ]; then \
		echo "SFML is available. You can build the GUI version."; \
	else \
		echo "SFML is not available. Install SFML development libraries."; \
	fi

.PHONY: all clean install-deps install-sfml run build debug check-sfml
