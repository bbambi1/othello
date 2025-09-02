#include "tournament_console.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

TournamentConsole::TournamentConsole() {
    initializeAvailableAgents();
}

void TournamentConsole::run() {
    while (true) {
        clearScreen();
        std::cout << "Tournament Setup" << std::endl;
        std::cout << "================" << std::endl;
        std::cout << std::endl;

        // 1) Select tournament type first
        configureTournamentType();

        // 2) Configure tournament parameters (time limit, rounds, etc.)
        configureTournament();

        // 3) Select agents (require at least 2)
        configureAgents();

        // 4) Run tournament
        runTournament();

        // 5) Save results as JSON for downstream analysis
        if (getYesNoInput("Save results as JSON? (y/n): ")) {
            saveResultsJson();
        }

        if (!getYesNoInput("Run another tournament setup? (y/n): ")) {
            std::cout << "Goodbye!" << std::endl;
            return;
        }
    }
}

void TournamentConsole::showMainMenu() {
    clearScreen();
    std::cout << "Othello Tournament System" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << std::endl;
    
    displayCurrentConfiguration();
    
    std::cout << std::endl;
    std::cout << "Main Menu:" << std::endl;
    std::cout << "1. Configure Agents" << std::endl;
    std::cout << "2. Configure Tournament" << std::endl;
    std::cout << "3. Run Tournament" << std::endl;
    std::cout << "4. Show Results" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << std::endl;
}

void TournamentConsole::showAgentSelectionMenu() {
    clearScreen();
    std::cout << "Agent Selection" << std::endl;
    std::cout << "===============" << std::endl;
    std::cout << std::endl;
    
    displayAvailableAgents();
    displaySelectedAgents();
    
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "1. Add Agent" << std::endl;
    std::cout << "2. Remove Agent" << std::endl;
    std::cout << "3. Clear All Agents" << std::endl;
    std::cout << "4. Back to Main Menu" << std::endl;
    std::cout << std::endl;
}

void TournamentConsole::showConfigurationMenu() {
    clearScreen();
    std::cout << "Tournament Configuration" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << std::endl;
    
    auto config = tournament_.getConfig();
    std::cout << "Current Settings:" << std::endl;
    std::cout << "  Type: " << config.tournamentType << std::endl;
    std::cout << "  Time Limit: " << config.timeLimit.count() << "ms" << std::endl;
    std::cout << "  Rounds per Matchup: " << config.roundsPerMatchup << std::endl;
    std::cout << "  Visual Feedback: " << (config.enableVisualFeedback ? "Yes" : "No") << std::endl;
    std::cout << "  Log Games: " << (config.logGames ? "Yes" : "No") << std::endl;
    std::cout << "  Log File: " << config.logFile << std::endl;
    
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "1. Set Tournament Type" << std::endl;
    std::cout << "2. Set Time Limit" << std::endl;
    std::cout << "3. Set Rounds per Matchup" << std::endl;
    std::cout << "4. Toggle Visual Feedback" << std::endl;
    std::cout << "5. Toggle Game Logging" << std::endl;
    std::cout << "6. Set Log File" << std::endl;
    std::cout << "7. Back" << std::endl;
    std::cout << std::endl;
}

void TournamentConsole::configureAgents() {
    while (true) {
        showAgentSelectionMenu();
        int choice = getMenuChoice(1, 4);
        
        switch (choice) {
            case 1: {
                std::cout << "Available Agent Types:" << std::endl;
                for (size_t i = 0; i < availableAgentTypes_.size(); ++i) {
                    std::cout << (i + 1) << ". " << availableAgentTypes_[i] << std::endl;
                }
                
                int agentChoice = getIntInput("Select agent type (1-" + std::to_string(availableAgentTypes_.size()) + "): ", 
                                             1, availableAgentTypes_.size());
                std::string agentType = availableAgentTypes_[agentChoice - 1];
                
                std::string customName = getStringInput("Enter custom name (or press Enter for default): ");
                if (customName.empty()) {
                    customName = agentType;
                }
                
                tournament_.addAgent(agentType, customName);
                std::cout << "Added agent: " << customName << " (" << agentType << ")" << std::endl;
                waitForKeyPress();
                break;
            }
            case 2: {
                // Remove agent functionality would go here
                std::cout << "Remove agent functionality not implemented yet." << std::endl;
                waitForKeyPress();
                break;
            }
            case 3: {
                if (getYesNoInput("Are you sure you want to clear all agents? (y/n): ")) {
                    tournament_.clearAgents();
                    std::cout << "All agents cleared." << std::endl;
                }
                waitForKeyPress();
                break;
            }
            case 4:
                return;
        }
    }
}

void TournamentConsole::configureTournament() {
    while (true) {
        showConfigurationMenu();
        int choice = getMenuChoice(1, 7);
        
        auto config = tournament_.getConfig();
        
        switch (choice) {
            case 1:
                configureTournamentType();
                break;
            case 2:
                configureTimeLimit();
                break;
            case 3:
                configureRounds();
                break;
            case 4:
                config.enableVisualFeedback = !config.enableVisualFeedback;
                tournament_.setConfig(config);
                std::cout << "Visual feedback " << (config.enableVisualFeedback ? "enabled" : "disabled") << std::endl;
                waitForKeyPress();
                break;
            case 5:
                config.logGames = !config.logGames;
                tournament_.setConfig(config);
                std::cout << "Game logging " << (config.logGames ? "enabled" : "disabled") << std::endl;
                waitForKeyPress();
                break;
            case 6: {
                std::string newLogFile = getStringInput("Enter log file name: ");
                if (!newLogFile.empty()) {
                    config.logFile = newLogFile;
                    tournament_.setConfig(config);
                    std::cout << "Log file set to: " << newLogFile << std::endl;
                }
                waitForKeyPress();
                break;
            }
            case 7:
                return;
        }
    }
}

void TournamentConsole::configureTournamentType() {
    auto config = tournament_.getConfig();
    std::cout << "Select Tournament Type" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << "1. Round Robin" << std::endl;
    int choice = getMenuChoice(1, 1);
    switch (choice) {
        case 1:
        default:
            config.tournamentType = "round_robin";
            break;
    }
    tournament_.setConfig(config);
}

void TournamentConsole::configureTimeLimit() {
    std::cout << "Current time limit: " << tournament_.getConfig().timeLimit.count() << "ms" << std::endl;
    int timeLimit = getIntInput("Enter time limit in milliseconds (100-30000): ", 100, 30000);
    
    auto config = tournament_.getConfig();
    config.timeLimit = std::chrono::milliseconds(timeLimit);
    tournament_.setConfig(config);
    
    std::cout << "Time limit set to " << timeLimit << "ms" << std::endl;
    waitForKeyPress();
}

void TournamentConsole::configureRounds() {
    std::cout << "Current rounds per matchup: " << tournament_.getConfig().roundsPerMatchup << std::endl;
    int rounds = getIntInput("Enter rounds per matchup (1-10): ", 1, 10);
    
    auto config = tournament_.getConfig();
    config.roundsPerMatchup = rounds;
    tournament_.setConfig(config);
    
    std::cout << "Rounds per matchup set to " << rounds << std::endl;
    waitForKeyPress();
}

void TournamentConsole::runTournament() {
    if (!validateConfiguration()) {
        waitForKeyPress();
        return;
    }
    
    std::cout << "Starting tournament..." << std::endl;
    std::cout << "Press Ctrl+C to interrupt (results will be saved)" << std::endl;
    waitForKeyPress();
    
    // Set up progress callback for visual feedback
    tournament_.setProgressCallback([](int /*current*/, int /*total*/, const std::string& /*currentMatch*/) {
        // Progress is already handled in the tournament class
    });
    
    try {
        auto config = tournament_.getConfig();
        if (config.tournamentType == "round_robin") {
            tournament_.runRoundRobin();
        } else {
            std::cout << "Unsupported tournament type: " << config.tournamentType << std::endl;
            return;
        }
        
        std::cout << std::endl;
        std::cout << "Tournament completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Tournament interrupted: " << e.what() << std::endl;
    }
    
    waitForKeyPress();
}

void TournamentConsole::saveResultsJson() {
    std::string filename = getStringInput("Enter JSON filename (or press Enter for 'tournament_results.json'): ");
    if (filename.empty()) {
        filename = "tournament_results.json";
    }
    tournament_.saveResultsJson(filename);
    waitForKeyPress();
}

void TournamentConsole::showResults() {
    clearScreen();
    std::cout << "Tournament Results" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << std::endl;
    
    if (tournament_.getGameResults().empty()) {
        std::cout << "No tournament results available." << std::endl;
        std::cout << "Run a tournament first." << std::endl;
    } else {
        tournament_.printResults();
    }
    
    std::cout << std::endl;
    waitForKeyPress();
}

void TournamentConsole::saveResults() {
    std::string filename = getStringInput("Enter filename (or press Enter for 'tournament_results.txt'): ");
    if (filename.empty()) {
        filename = "tournament_results.txt";
    }
    
    tournament_.saveResults(filename);
    waitForKeyPress();
}

void TournamentConsole::clearScreen() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void TournamentConsole::waitForKeyPress() const {
    std::cout << "Press Enter to continue...";
    
    // Clear any error flags
    std::cin.clear();
    
    // Use getline to wait for user input (more reliable)
    std::string dummy;
    std::getline(std::cin, dummy);
}

int TournamentConsole::getMenuChoice(int min, int max) const {
    int choice;
    while (true) {
        std::cout << "Enter your choice (" << min << "-" << max << "): ";
        std::cin >> choice;
        
        if (std::cin.fail() || choice < min || choice > max) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid choice. Please try again." << std::endl;
        } else {
            std::cin.ignore(10000, '\n');
            return choice;
        }
    }
}

std::string TournamentConsole::getStringInput(const std::string& prompt) const {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int TournamentConsole::getIntInput(const std::string& prompt, int min, int max) const {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        
        if (std::cin.fail() || value < min || value > max) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a number between " << min << " and " << max << "." << std::endl;
        } else {
            // Clear the newline after successful input
            std::cin.ignore(10000, '\n');
            return value;
        }
    }
}

bool TournamentConsole::getYesNoInput(const std::string& prompt) const {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        
        // Convert to lowercase
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        
        if (input == "y" || input == "yes") {
            return true;
        } else if (input == "n" || input == "no") {
            return false;
        } else {
            std::cout << "Please enter 'y' or 'n'." << std::endl;
        }
    }
}

void TournamentConsole::displayCurrentConfiguration() const {
    auto config = tournament_.getConfig();
    auto agentNames = tournament_.getAgentNames();
    
    std::cout << "Current Configuration:" << std::endl;
    std::cout << "  Agents: " << agentNames.size() << std::endl;
    std::cout << "  Time Limit: " << config.timeLimit.count() << "ms" << std::endl;
    std::cout << "  Rounds per Matchup: " << config.roundsPerMatchup << std::endl;
    std::cout << "  Visual Feedback: " << (config.enableVisualFeedback ? "Yes" : "No") << std::endl;
    std::cout << "  Log Games: " << (config.logGames ? "Yes" : "No") << std::endl;
}

void TournamentConsole::displayAvailableAgents() const {
    std::cout << "Available Agent Types:" << std::endl;
    for (size_t i = 0; i < availableAgentTypes_.size(); ++i) {
        std::string displayName = availableAgentTypes_[i];
        // Capitalize first letter for display
        if (!displayName.empty()) {
            displayName[0] = std::toupper(displayName[0]);
        }
        std::cout << "  " << (i + 1) << ". " << displayName << std::endl;
    }
}

void TournamentConsole::displaySelectedAgents() const {
    auto agentNames = tournament_.getAgentNames();
    
    std::cout << std::endl;
    std::cout << "Selected Agents (" << agentNames.size() << "):" << std::endl;
    if (agentNames.empty()) {
        std::cout << "  No agents selected" << std::endl;
    } else {
        for (const auto& name : agentNames) {
            std::cout << "  - " << name << std::endl;
        }
    }
}

bool TournamentConsole::validateConfiguration() const {
    auto agentNames = tournament_.getAgentNames();
    
    if (agentNames.size() < 2) {
        std::cout << "Error: Need at least 2 agents for a tournament." << std::endl;
        std::cout << "Current agents: " << agentNames.size() << std::endl;
        return false;
    }
    
    return true;
}

void TournamentConsole::initializeAvailableAgents() {
    // These should match the registered agent types in the system (lowercase)
    availableAgentTypes_ = {
        "random",
        "greedy", 
        "minmax"
    };
}
