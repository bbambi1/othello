#include "tournament_console.h"
#include <iostream>
#include <exception>

int main() {
    try {
        TournamentConsole console;
        console.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
