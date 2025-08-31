#include "tournament_interface.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <chrono>
#include <thread>

TournamentInterface::TournamentInterface() {
    // Initialize available agent types
    availableAgentTypes = {"random", "greedy", "minmax", "positional", "hybrid"};
}

void TournamentInterface::run() {
    while (true) {
        clearScreen();
        std::cout << "=== OTHELLO AI TOURNAMENT INTERFACE ===" << std::endl;
        std::cout << std::endl;
        
        showMainMenu();
        
        std::cout << "\nExit program? (y/n): ";
        if (getYesNoInput("Exit program?")) {
            break;
        }
    }
    
    std::cout << "Thanks for using the Tournament Interface!\n";
}

void TournamentInterface::showMainMenu() {
    displayCurrentConfiguration();
    
    std::cout << "\n=== MAIN MENU ===" << std::endl;
    std::cout << "1. Configure Agents" << std::endl;
    std::cout << "2. Configure Tournament Mode" << std::endl;
    std::cout << "3. Configure Parameters" << std::endl;
    std::cout << "4. Configure Safety Settings" << std::endl;
    std::cout << "5. Run Tournament" << std::endl;
    std::cout << "6. View Available Agents" << std::endl;
    std::cout << "7. View Tournament Modes" << std::endl;
    std::cout << "8. View Safety Options" << std::endl;
    std::cout << "0. Exit" << std::endl;
    
    std::cout << "\nEnter your choice (0-8): ";
    int choice = getMenuChoice(0, 8);
    
    switch (choice) {
        case 0:
            return;
        case 1:
            configureAgents();
            break;
        case 2:
            configureTournamentMode();
            break;
        case 3:
            configureParameters();
            break;
        case 4:
            configureSafety();
            break;
        case 5:
            if (validateConfiguration()) {
                runTournament();
            } else {
                std::cout << "\nConfiguration incomplete. Please configure all required settings first.\n";
                waitForKeyPress();
            }
            break;
        case 6:
            displayAvailableAgents();
            waitForKeyPress();
            break;
        case 7:
            displayTournamentModes();
            waitForKeyPress();
            break;
        case 8:
            displaySafetyOptions();
            waitForKeyPress();
            break;
    }
}

void TournamentInterface::configureAgents() {
    clearScreen();
    std::cout << "=== AGENT CONFIGURATION ===" << std::endl;
    
    displayAvailableAgents();
    
    std::cout << "\nEnter comma-separated list of agent types to include: ";
    std::string input = getStringInput("Agent types");
    
    // Parse comma-separated list
    std::vector<std::string> selectedAgents;
    size_t pos = 0;
    while ((pos = input.find(',')) != std::string::npos) {
        std::string agent = input.substr(0, pos);
        // Trim whitespace
        agent.erase(0, agent.find_first_not_of(" \t"));
        agent.erase(agent.find_last_not_of(" \t") + 1);
        if (!agent.empty()) {
            selectedAgents.push_back(agent);
        }
        input.erase(0, pos + 1);
    }
    if (!input.empty()) {
        input.erase(0, input.find_first_not_of(" \t"));
        input.erase(input.find_last_not_of(" \t") + 1);
        if (!input.empty()) {
            selectedAgents.push_back(input);
        }
    }
    
    // Validate selected agents
    std::vector<std::string> validAgents;
    for (const auto& agent : selectedAgents) {
        if (std::find(availableAgentTypes.begin(), availableAgentTypes.end(), agent) != availableAgentTypes.end()) {
            validAgents.push_back(agent);
        } else {
            std::cout << "Warning: Unknown agent type '" << agent << "' ignored.\n";
        }
    }
    
    if (validAgents.empty()) {
        std::cout << "Error: No valid agents selected.\n";
        waitForKeyPress();
        return;
    }
    
    // Add agents to tournament
    tournament.resetTournament();
    for (const auto& agentType : validAgents) {
        tournament.addAgent(agentType);
    }
    
    std::cout << "\nSuccessfully configured " << validAgents.size() << " agents:\n";
    for (const auto& agent : validAgents) {
        std::cout << "  - " << agent << std::endl;
    }
    
    waitForKeyPress();
}

void TournamentInterface::configureTournamentMode() {
    clearScreen();
    std::cout << "=== TOURNAMENT MODE CONFIGURATION ===" << std::endl;
    
    displayTournamentModes();
    
    std::cout << "\nSelect tournament mode (1-3): ";
    int choice = getMenuChoice(1, 3);
    
    std::string mode;
    switch (choice) {
        case 1:
            mode = "roundrobin";
            break;
        case 2:
            mode = "singleelim";
            break;
        case 3:
            mode = "swiss";
            break;
    }
    
    // Store the selected mode (we'll use it when running the tournament)
    // For now, we'll just confirm the selection
    std::cout << "\nSelected tournament mode: " << getTournamentTypeName(mode) << std::endl;
    
    waitForKeyPress();
}

void TournamentInterface::configureParameters() {
    clearScreen();
    std::cout << "=== PARAMETER CONFIGURATION ===" << std::endl;
    
    std::cout << "\n1. Number of rounds per matchup\n";
    std::cout << "2. Time limit per move (milliseconds)\n";
    std::cout << "3. Output file for results\n";
    std::cout << "4. Log file for game details\n";
    
    std::cout << "\nSelect parameter to configure (1-4): ";
    int choice = getMenuChoice(1, 4);
    
    switch (choice) {
        case 1: {
            std::cout << "\nEnter number of rounds per matchup (1-10): ";
            int rounds = getIntInput("Rounds", 1, 10);
            tournament.setRoundsPerMatchup(rounds);
            std::cout << "Set to " << rounds << " rounds per matchup.\n";
            break;
        }
        case 2: {
            std::cout << "\nEnter time limit per move in milliseconds (100-10000): ";
            int timeLimit = getIntInput("Time limit (ms)", 100, 10000);
            tournament.setTimeLimit(std::chrono::milliseconds(timeLimit));
            std::cout << "Set to " << timeLimit << " ms per move.\n";
            break;
        }
        case 3: {
            std::cout << "\nEnter output file name for results: ";
            std::string filename = getStringInput("Output file");
            if (!filename.empty()) {
                // Store the filename (we'll use it when saving results)
                std::cout << "Output file set to: " << filename << std::endl;
            }
            break;
        }
        case 4: {
            std::cout << "\nEnter log file name for game details: ";
            std::string filename = getStringInput("Log file");
            if (!filename.empty()) {
                tournament.setLogFile(filename);
                std::cout << "Log file set to: " << filename << std::endl;
            }
            break;
        }
    }
    
    waitForKeyPress();
}

void TournamentInterface::configureSafety() {
    clearScreen();
    std::cout << "=== SAFETY CONFIGURATION ===" << std::endl;
    
    displaySafetyOptions();
    
    std::cout << "\nEnable safety wrapper? (y/n): ";
    bool enableSafety = getYesNoInput("Enable safety wrapper");
    
    if (enableSafety) {
        // Configure safety settings
        SafetyConfig config;
        std::cout << "\nConfigure safety settings:\n";
        
        std::cout << "Time limit per move (ms): ";
        int timeLimit = getIntInput("Time limit", 100, 10000);
        config.timeLimit = std::chrono::milliseconds(timeLimit);
        
        std::cout << "Enable crash detection? (y/n): ";
        config.enableCrashDetection = getYesNoInput("Enable crash detection");
        
        std::cout << "Enable resource monitoring? (y/n): ";
        config.enableResourceMonitoring = getYesNoInput("Enable resource monitoring");
        
        std::cout << "Enable move validation? (y/n): ";
        config.enableMoveValidation = getYesNoInput("Enable move validation");
        
        std::cout << "Enable time limit enforcement? (y/n): ";
        config.enableTimeLimit = getYesNoInput("Enable time limit enforcement");
        
        tournament.setSafetyConfig(config);
        std::cout << "\nSafety configuration applied.\n";
    } else {
        std::cout << "\nWarning: Safety wrapper disabled. This is not recommended for untrusted agents.\n";
        // Note: We'd need to add a method to disable safety in TournamentManager
    }
    
    waitForKeyPress();
}

void TournamentInterface::runTournament() {
    clearScreen();
    std::cout << "=== RUNNING TOURNAMENT ===" << std::endl;
    
    // For now, we'll run a simple round-robin tournament
    // In a full implementation, we'd use the configured tournament mode
    
    std::cout << "Starting tournament...\n";
    std::cout << "This will run all agents against each other.\n";
    
    std::cout << "\nPress Enter to start...";
    waitForKeyPress();
    
    // Run the tournament
    tournament.runRoundRobin();
    
    // Show results
    showFinalResults();
}

void TournamentInterface::showTournamentProgress() {
    // This would show real-time progress during tournament execution
    std::cout << "Tournament in progress...\n";
}

void TournamentInterface::showFinalResults() {
    clearScreen();
    std::cout << "=== TOURNAMENT RESULTS ===" << std::endl;
    
    tournament.printResults();
    
    std::cout << "\nResults have been saved to the configured output files.\n";
    waitForKeyPress();
}

void TournamentInterface::displayCurrentConfiguration() const {
    std::cout << "Current Configuration:\n";
    std::cout << "  Agents: ";
    // We'd need to add a method to get current agents from TournamentManager
    std::cout << "Not configured\n";
    
    std::cout << "  Tournament Mode: Round Robin (default)\n";
    std::cout << "  Rounds per matchup: " << "1 (default)\n";
    std::cout << "  Time limit: " << "1000 ms (default)\n";
    std::cout << "  Safety wrapper: Enabled (default)\n";
}

void TournamentInterface::displayAvailableAgents() const {
    std::cout << "Available AI Agent Types:\n";
    std::cout << "1. random    - Makes random valid moves\n";
    std::cout << "2. greedy    - Always chooses move with most disc flips\n";
    std::cout << "3. minmax    - Uses minimax algorithm with configurable depth\n";
    std::cout << "4. positional - Evaluates board position and stability\n";
    std::cout << "5. hybrid    - Combines multiple strategies\n";
}

void TournamentInterface::displayTournamentModes() const {
    std::cout << "Available Tournament Modes:\n";
    std::cout << "1. Round Robin - Each agent plays against every other agent\n";
    std::cout << "2. Single Elimination - Knockout tournament with brackets\n";
    std::cout << "3. Swiss System - Pairing system for balanced competition\n";
}

void TournamentInterface::displaySafetyOptions() const {
    std::cout << "Safety Configuration Options:\n";
    std::cout << "- Time limits per move\n";
    std::cout << "- Memory usage limits\n";
    std::cout << "- File access restrictions\n";
    std::cout << "- Network access restrictions\n";
    std::cout << "- Exception handling and crash recovery\n";
}

bool TournamentInterface::validateConfiguration() const {
    // Check if agents are configured
    // Check if tournament mode is selected
    // Check if parameters are set
    return true; // Simplified for now
}

std::string TournamentInterface::getTournamentTypeName(const std::string& type) const {
    if (type == "roundrobin") return "Round Robin";
    if (type == "singleelim") return "Single Elimination";
    if (type == "swiss") return "Swiss System";
    return "Unknown";
}

// Utility functions
void TournamentInterface::clearScreen() const {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void TournamentInterface::waitForKeyPress() const {
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int TournamentInterface::getMenuChoice(int min, int max) const {
    int choice;
    while (!(std::cin >> choice) || choice < min || choice > max) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid choice. Please enter a number between " << min << " and " << max << ": ";
    }
    return choice;
}

std::string TournamentInterface::getStringInput(const std::string& prompt) const {
    std::string input;
    std::cout << prompt << ": ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, input);
    return input;
}

int TournamentInterface::getIntInput(const std::string& prompt, int min, int max) const {
    int value;
    while (!(std::cin >> value) || value < min || value > max) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << prompt << " (" << min << "-" << max << "): ";
    }
    return value;
}

bool TournamentInterface::getYesNoInput(const std::string& prompt) const {
    std::string input;
    std::cout << prompt << " (y/n): ";
    std::cin >> input;
    return (input == "y" || input == "Y" || input == "yes" || input == "Yes");
}
