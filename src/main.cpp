#include <iostream>
#include <string>
#include "grid.h"

static void printUsage() {
    std::cout << "\033[1mConnect the Dots\033[0m \u2014 Terminal Flow Puzzle\n\n"
              << "Usage: ./connectthedots [command] [options]\n\n"
              << "Commands:\n"
              << " play <file>                      Play a interactive puzzle\n"
              << " solve <file> [--animate]         Solve a puzzle with backtracking\n"
              << " generate <W> <H> <flows>         Generate a new puzzle\n"
              << " stats                            Show solve statistics\n\n"
              << " Examples:\n"
              << " ./connectthedots play puzzles/5x5_easy.txt\n"
              << " ./connectthedots solve puzzles/7x7_hard.txt --animate\n"
              << " ./connectthedots generate 8 8 6\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "play") {
        if (argc < 3) {
            std::cerr << "Usage: ./connectthedots play <file>\n";
            return 1;
        }
        Grid grid;
        if (!grid.loadFromFile(argv[2])) {
            std::cerr << "Error: Could not load puzzle from" << argv[2] << "\n";
            return 1;
        }
        printf("\nLoaded %dx%d puzzle with %d flows\n\n", grid.width, grid.height, grid.numColors);
    }

    return 0;
}