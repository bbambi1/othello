#include "tournament_manager.h"
#include <iostream>
#include <string>

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --help              - Show this help message\n";
    std::cout << "  --agents <list>     - Comma-separated list of AI agent types\n";
    std::cout << "  --tournament <type> - Tournament type: roundrobin, singleelim, swiss\n";
    std::cout << "  --rounds <number>   - Number of rounds per matchup (default: 1)\n";
    std::cout << "  --output <file>     - Output file for results (default: tournament_results.txt)\n";
    std::cout << "  --log <file>        - Log file for game details (default: tournament.log)\n";
    std::cout << "\nAvailable AI agent types:\n";
    std::cout << "  random, greedy, minmax, positional, hybrid\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --agents random,greedy,minmax --tournament roundrobin\n";
    std::cout << "  " << programName << " --agents minmax,hybrid --tournament singleelim\n";
    std::cout << "  " << programName << " --agents random,greedy,minmax,positional,hybrid --tournament swiss --rounds 3\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> agentTypes = {"random", "greedy", "minmax"};
    std::string tournamentType = "roundrobin";
    int rounds = 1;
    std::string outputFile = "tournament_results.txt";
    std::string logFile = "tournament.log";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--agents" && i + 1 < argc) {
            std::string agents = argv[++i];
            agentTypes.clear();
            size_t pos = 0;
            while ((pos = agents.find(',')) != std::string::npos) {
                agentTypes.push_back(agents.substr(0, pos));
                agents.erase(0, pos + 1);
            }
            if (!agents.empty()) {
                agentTypes.push_back(agents);
            }
        } else if (arg == "--tournament" && i + 1 < argc) {
            tournamentType = argv[++i];
        } else if (arg == "--rounds" && i + 1 < argc) {
            rounds = std::stoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--log" && i + 1 < argc) {
            logFile = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Validate tournament type
    if (tournamentType != "roundrobin" && tournamentType != "singleelim" && tournamentType != "swiss") {
        std::cerr << "Invalid tournament type: " << tournamentType << std::endl;
        std::cerr << "Valid types: roundrobin, singleelim, swiss" << std::endl;
        return 1;
    }
    
    try {
        std::cout << "=== Othello AI Tournament Runner ===" << std::endl;
        std::cout << "Tournament Type: " << tournamentType << std::endl;
        std::cout << "AI Agents: ";
        for (size_t i = 0; i < agentTypes.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << agentTypes[i];
        }
        std::cout << std::endl;
        std::cout << "Rounds: " << rounds << std::endl;
        std::cout << "Output File: " << outputFile << std::endl;
        std::cout << "Log File: " << logFile << std::endl;
        std::cout << std::endl;
        
        // Create tournament manager
        TournamentManager tournament;
        tournament.setLogFile(logFile);
        
        // Add AI agents
        for (const auto& agentType : agentTypes) {
            tournament.addAgent(agentType);
            std::cout << "Added AI agent: " << agentType << std::endl;
        }
        
        // Run tournament
        std::cout << "\nStarting tournament..." << std::endl;
        
        if (tournamentType == "roundrobin") {
            tournament.runRoundRobin(rounds);
        } else if (tournamentType == "singleelim") {
            tournament.runSingleElimination();
        } else if (tournamentType == "swiss") {
            tournament.runSwissSystem(rounds);
        }
        
        // Display and save results
        std::cout << "\nTournament completed!" << std::endl;
        tournament.printResults();
        tournament.saveResults(outputFile);
        tournament.saveGameLogs("game_logs.txt");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
