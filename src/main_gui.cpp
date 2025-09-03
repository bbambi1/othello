#include <iostream>
#include <string>
#include <memory>

#ifdef USE_SFML
#include "gui_interface.h"
#endif

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [interface] [options]\n";
    std::cout << "Interfaces:\n";
#ifdef USE_SFML
    std::cout << "  gui        - Graphical user interface (requires SFML)\n";
#endif
    std::cout << "\nOptions:\n";
    std::cout << "  --help     - Show this help message\n";
    std::cout << "  --version  - Show version information\n";
    std::cout << "\nExamples:\n";
#ifdef USE_SFML
    std::cout << "  " << programName << " gui          # Launch GUI interface\n";
#endif
}

void printVersion() {
    std::cout << "Othello/Reversi Game v2.0\n";
    std::cout << "A complete implementation with AI agents and GUI interface\n";
#ifdef USE_SFML
    std::cout << "Built with SFML support for GUI\n";
#else
    std::cout << "GUI interface not available (rebuild with SFML for GUI support)\n";
#endif
}

int main(int argc, char* argv[]) {
    std::string interface = "gui"; // Default to GUI for this executable

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        } else if (arg == "gui") {
#ifdef USE_SFML
            interface = "gui";
#else
            std::cerr << "Error: GUI interface not available. Rebuild with SFML support.\n";
            return 1;
#endif
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    try {
#ifdef USE_SFML
        if (interface == "gui") {
            std::cout << "Launching GUI Interface...\n";
            GUIInterface gui;
            gui.run();
        }
#else
        std::cerr << "Error: GUI interface not available. Rebuild with SFML support.\n";
        return 1;
#endif
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}
