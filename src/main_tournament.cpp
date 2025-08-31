#include "tournament_interface.h"
#include <iostream>
#include <exception>

int main() {
    try {
        std::cout << "=== OTHELLO AI TOURNAMENT SYSTEM ===" << std::endl;
        std::cout << "Interactive Tournament Interface" << std::endl;
        std::cout << std::endl;
        
        TournamentInterface interface;
        interface.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
