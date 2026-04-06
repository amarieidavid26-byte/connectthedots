#include <iostream>
#include <string>
#include "grid.h"
#include "display.h"
#include "solver.h"

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
        Display::drawGrid(grid, argv[2]);
        std::cout << "\nPress any key to start playing...\n";
    
    } else if (cmd == "solve") {
        if (argc < 3) {
            std::cerr << "Usage: ./connectthedots solve <file> [--animate]\n";
            return 1;
        }
        Grid grid;
        if (!grid.loadFromFile(argv[2])) {
            std::cerr << "Error: Could not load puzzle from " << argv[2] << "\n";
            return 1;
        }

        printf("\n\033[1mPuzzle:\033[0m %s (%dx%dx, %d flows)\n\n", argv[2], grid.width, grid.height, grid.numColors);
        Display::drawGrid(grid);

        Solver solver;
        for (int i = 3; i < argc; i++)
            if (std::string(argv[i]) == "--animate")
                solver.animate = true;
            
        std::cout << "\nSolving...\n";
        auto result = solver.solve(grid);

        if (result.solved) {
            std::cout << "\n";
            Display::drawGrid(result.solution, "Solution:");
            printf("\n Solved in %.1f ms | %d backtracks | Difficulty: %s\n\n",
                    result.timeMs, result.backtracks, result.difficulty.c_str());
        } else {
            std::cout << "\n No solution found.\n\n";
        }
    }

    return 0;
}