#include "tournament_console.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

TournamentConsole::TournamentConsole() {
  initializeAvailableAgents();

  auto config = tournament_.getConfig();
  config.timeLimit = std::chrono::milliseconds(1000);
  config.roundsPerMatchup = 3;
  config.enableVisualFeedback = true;
  config.logGames = true;
  config.tournamentType = "round_robin";
  tournament_.setConfig(config);
}

void TournamentConsole::run() {
  while (true) {
    clearScreen();
    std::cout << "Othello Tournament Setup" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << std::endl;

    configureTournamentType();

    configureTimeLimit();

    configureRounds();

    setupAllAgents();

    runTournament();

    saveResultsJson();

    if (!getYesNoInput("Run another tournament? (y/n): ")) {
      std::cout << "Goodbye!" << std::endl;
      return;
    }
  }
}

void TournamentConsole::showMainMenu() {
  clearScreen();
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
  std::cout << "  Time Limit: " << config.timeLimit.count() << "ms"
            << std::endl;
  std::cout << "  Rounds per Matchup: " << config.roundsPerMatchup << std::endl;
  std::cout << "  Visual Feedback: "
            << (config.enableVisualFeedback ? "Yes" : "No") << std::endl;
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

      int agentChoice =
          getIntInput("Select agent type (1-" +
                          std::to_string(availableAgentTypes_.size()) + "): ",
                      1, availableAgentTypes_.size());
      std::string agentType = availableAgentTypes_[agentChoice - 1];

      std::string customName =
          getStringInput("Enter custom name (or press Enter for default): ");
      if (customName.empty()) {
        customName = agentType;
      }

      tournament_.addAgent(agentType, customName);
      std::cout << "Added agent: " << customName << " (" << agentType << ")"
                << std::endl;
      waitForKeyPress();
      break;
    }
    case 2: {
      std::cout << "Remove agent functionality not implemented yet."
                << std::endl;
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
      std::cout << "Visual feedback "
                << (config.enableVisualFeedback ? "enabled" : "disabled")
                << std::endl;
      waitForKeyPress();
      break;
    case 5:
      config.logGames = !config.logGames;
      tournament_.setConfig(config);
      std::cout << "Game logging " << (config.logGames ? "enabled" : "disabled")
                << std::endl;
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
  std::cout << "1. Select Tournament Type" << std::endl;
  std::cout << "   Round Robin (all agents play each other)" << std::endl;
  std::cout << "   Press Enter to continue...";
  std::string dummy;
  std::getline(std::cin, dummy);

  config.tournamentType = "round_robin";
  tournament_.setConfig(config);
  std::cout << "   ✓ Tournament type: Round Robin" << std::endl;
  std::cout << std::endl;
}

void TournamentConsole::configureTimeLimit() {
  auto config = tournament_.getConfig();
  std::cout << "2. Set Time Limit per Move" << std::endl;
  std::cout << "   Default: 1000ms" << std::endl;
  std::cout << "   Enter time limit in milliseconds (100-30000) or press Enter "
               "for default: ";

  std::string input;
  std::getline(std::cin, input);

  int timeLimit = 1000; // Default
  if (!input.empty()) {
    try {
      timeLimit = std::stoi(input);
      if (timeLimit < 100 || timeLimit > 30000) {
        std::cout << "   Invalid range, using default 1000ms" << std::endl;
        timeLimit = 1000;
      }
    } catch (...) {
      std::cout << "   Invalid input, using default 1000ms" << std::endl;
      timeLimit = 1000;
    }
  }

  config.timeLimit = std::chrono::milliseconds(timeLimit);
  tournament_.setConfig(config);

  std::cout << "   ✓ Time limit: " << timeLimit << "ms" << std::endl;
  std::cout << std::endl;
}

void TournamentConsole::configureRounds() {
  auto config = tournament_.getConfig();
  std::cout << "3. Set Number of Rounds" << std::endl;
  std::cout << "   Note: 1 round = 2 games per matchup (each agent plays both "
               "black and white)"
            << std::endl;
  std::cout << "   Default: 3 rounds" << std::endl;
  std::cout << "   Enter number of rounds (1-10) or press Enter for default: ";

  std::string input;
  std::getline(std::cin, input);

  int rounds = 3; // Default
  if (!input.empty()) {
    try {
      rounds = std::stoi(input);
      if (rounds < 1 || rounds > 100) {
        std::cout << "   Invalid range, using default 3 rounds" << std::endl;
        rounds = 3;
      }
    } catch (...) {
      std::cout << "   Invalid input, using default 3 rounds" << std::endl;
      rounds = 3;
    }
  }

  config.roundsPerMatchup = rounds;
  tournament_.setConfig(config);

  std::cout << "   ✓ Rounds per matchup: " << rounds
            << " (each side plays both black and white)" << std::endl;
  std::cout << std::endl;
}

void TournamentConsole::setupAllAgents() {
  // Clear any existing agents
  tournament_.clearAgents();

  std::cout << "4. Adding All Available Agents" << std::endl;

  // Add all available agent types
  for (const auto &agentType : availableAgentTypes_) {
    tournament_.addAgent(agentType, agentType);
    std::cout << "   ✓ Added: " << agentType << std::endl;
  }

  std::cout << "   Total agents: " << availableAgentTypes_.size() << std::endl;
  std::cout << std::endl;
}

void TournamentConsole::runTournament() {
  if (!validateConfiguration()) {
    waitForKeyPress();
    return;
  }

  // Ensure visual feedback and logging are always enabled
  auto config = tournament_.getConfig();
  config.enableVisualFeedback = true;
  config.logGames = true;
  tournament_.setConfig(config);

  std::cout << "5. Running Tournament" << std::endl;
  std::cout << "   Press Ctrl+C to interrupt (results will be saved)"
            << std::endl;
  std::cout << "   Starting in 3 seconds...";
  std::cout.flush();

  // Countdown
  for (int i = 3; i > 0; i--) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << " " << i;
    std::cout.flush();
  }
  std::cout << std::endl << std::endl;

  // Set up progress callback for visual feedback
  tournament_.setProgressCallback(
      [](int /*current*/, int /*total*/, const std::string & /*currentMatch*/) {
        // Progress is already handled in the tournament class
      });

  try {
    if (config.tournamentType == "round_robin") {
      tournament_.runRoundRobin();
    } else {
      std::cout << "Unsupported tournament type: " << config.tournamentType
                << std::endl;
      return;
    }

    std::cout << std::endl;
    std::cout << "   ✓ Tournament completed successfully!" << std::endl;
    std::cout << std::endl;

  } catch (const std::exception &e) {
    std::cout << "   ✗ Tournament interrupted: " << e.what() << std::endl;
    std::cout << std::endl;
  }
}

void TournamentConsole::saveResultsJson() {
  std::cout << "6. Saving Results" << std::endl;

  // Generate timestamp-based filename
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto tm = *std::localtime(&time_t);

  std::ostringstream oss;
  oss << "tournament_results_" << std::put_time(&tm, "%Y%m%d_%H%M%S")
      << ".json";

  std::string filename = oss.str();
  tournament_.saveResultsJson(filename);

  std::cout << "   ✓ Results saved to: " << filename << std::endl;
  std::cout << std::endl;
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
  std::string filename = getStringInput(
      "Enter filename (or press Enter for 'tournament_results.txt'): ");
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

std::string TournamentConsole::getStringInput(const std::string &prompt) const {
  std::string input;
  std::cout << prompt;
  std::getline(std::cin, input);
  return input;
}

int TournamentConsole::getIntInput(const std::string &prompt, int min,
                                   int max) const {
  int value;
  while (true) {
    std::cout << prompt;
    std::cin >> value;

    if (std::cin.fail() || value < min || value > max) {
      std::cin.clear();
      std::cin.ignore(10000, '\n');
      std::cout << "Invalid input. Please enter a number between " << min
                << " and " << max << "." << std::endl;
    } else {
      // Clear the newline after successful input
      std::cin.ignore(10000, '\n');
      return value;
    }
  }
}

bool TournamentConsole::getYesNoInput(const std::string &prompt) const {
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
  std::cout << "  Time Limit: " << config.timeLimit.count() << "ms"
            << std::endl;
  std::cout << "  Rounds per Matchup: " << config.roundsPerMatchup << std::endl;
}

void TournamentConsole::displayAvailableAgents() const {
  std::cout << "Available Agent Types:" << std::endl;
  for (size_t i = 0; i < availableAgentTypes_.size(); ++i) {
    std::string displayName = availableAgentTypes_[i];
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
    for (const auto &name : agentNames) {
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
  availableAgentTypes_ = {"bitboard", "mcts", "panda", "plagiatBot",
                          "agentDuJardin"};
}
