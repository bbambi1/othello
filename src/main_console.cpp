#include "console_game.h"
#include <exception>
#include <iostream>

int main() {
  try {
    ConsoleGame game;
    game.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown error occurred" << std::endl;
    return 1;
  }

  return 0;
}
