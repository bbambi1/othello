#include <iostream>
#include <string>
#include <memory>

#ifdef USE_SFML
#include "gui_interface.h"
#endif

int main() {
    std::string interface = "gui";

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
