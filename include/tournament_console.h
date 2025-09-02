#pragma once

#include "simple_tournament.h"
#include <memory>
#include <vector>
#include <string>

class TournamentConsole {
public:
    TournamentConsole();
    ~TournamentConsole() = default;
    
    // Main interface
    void run();
    
private:
    SimpleTournament tournament_;
    
    // Available agent types
    std::vector<std::string> availableAgentTypes_;
    
    // Menu functions
    void showMainMenu();
    void showAgentSelectionMenu();
    void showConfigurationMenu();
    void showTournamentMenu();
    
    // Configuration functions
    void configureAgents();
    void configureTournament();
    void configureTournamentType();
    void configureTimeLimit();
    void configureRounds();
    
    // Tournament execution
    void runTournament();
    void showResults();
    void saveResults();
    void saveResultsJson();
    
    // Utility functions
    void clearScreen() const;
    void waitForKeyPress() const;
    int getMenuChoice(int min, int max) const;
    std::string getStringInput(const std::string& prompt) const;
    int getIntInput(const std::string& prompt, int min, int max) const;
    bool getYesNoInput(const std::string& prompt) const;
    
    // Display functions
    void displayCurrentConfiguration() const;
    void displayAvailableAgents() const;
    void displaySelectedAgents() const;
    
    // Validation functions
    bool validateConfiguration() const;
    void initializeAvailableAgents();
};
