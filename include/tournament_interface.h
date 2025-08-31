#pragma once

#include "tournament_manager.h"
#include <memory>
#include <vector>
#include <string>

class TournamentInterface {
public:
    TournamentInterface();
    
    // Main interface
    void run();
    
private:
    TournamentManager tournament;
    
    // Available agent types
    std::vector<std::string> availableAgentTypes;
    
    // Menu functions
    void showMainMenu();
    void showAgentSelectionMenu();
    void showTournamentModeMenu();
    void showConfigurationMenu();
    void showSafetyMenu();
    
    // Configuration functions
    void configureAgents();
    void configureTournamentMode();
    void configureParameters();
    void configureSafety();
    
    // Tournament execution
    void runTournament();
    void showTournamentProgress();
    void showFinalResults();
    
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
    void displayTournamentModes() const;
    void displaySafetyOptions() const;
    
    // Validation functions
    bool validateConfiguration() const;
    std::string getTournamentTypeName(const std::string& type) const;
};
