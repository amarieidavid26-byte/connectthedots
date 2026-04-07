#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include "grid.h"
#include "display.h"
#include "solver.h"
#include "generator.h"

static struct termios origtermios;

static void enableRawMode() {
    tcgetattr(STDIN_FILENO, &origtermios);
    struct termios raw = origtermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origtermios);
}

static int readKey() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;

    if (c== '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';
        if(seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'w';
                case 'B': return 's';
                case 'C': return 'd';
                case 'D': return 'a';
            }
        }
        return '\033';
    }
    return c;
}

static void updateStats(double ms, int bt, const std::string& diff, int sz) {
    std::ofstream file ("stats.txt", std::ios::app);
    if (file.is_open())
        file << sz << " " << ms << " " << bt << " " << diff << "\n";
}

static void showStats() {
    std::ifstream file("stats.txt");
    if (!file.is_open()) {
        std::cout << "No stats recorded yet.\n";
        return;
    }

    int total = 0;
    double totalTime = 0;
    long long totalBt = 0;
    int sz; double ms; int bt; std::string diff;

    std::cout << "\033[1m=== Solve statistics ===\033[0m\n\n";
    std::cout << "   Size |   Time (ms)   |  Backtracks  |  Difficulty  |\n";
    std::cout << " -------+---------------+--------------+---------------\n";

    while (file >> sz >> ms >> bt >> diff) {
        printf(" %3dx%-2d | %10.1f  | %8d  | %s\n", sz, sz, ms, bt, diff.c_str());
        total++;
        totalTime += ms;
        totalBt += bt;
    }

    if (total > 0) {
        std::cout << " ------+--------------+---------------+----------------\n";
        printf( " Totals: %d solved, avg %.1f ms, avg %lld backtracks\n",
                total, totalTime / total, totalBt / total);
    }
    std::cout << "\n";
}

static void playInteractive(Grid& grid) {
    enableRawMode();

    int cur = 1;
    int moves = 0;
    auto t0 = std::chrono::high_resolution_clock::now();
    
    Grid working = grid.clone();
    std::vector<std::vector<std::pair<int,int>>> paths(grid.numColors + 1);
    for (auto& f : working.flows) {
        paths[f.color].clear();
        paths[f.color].push_back(f.start);
    }

    auto redraw = [&]() {
        Display::clearScreen();
        Display::drawGrid(working, "Connect the dots \u2014 Interactive mode");
        std::cout << "\n Current flow: " << Display::colorCode(cur)
                  << Display::boldCode() << (char)('A' + cur - 1)
                  << Display::resetCode() << "\n";
        std::cout << " WASD/Arrows: move | 1-" << grid.numColors<< ": select flow | U: undo | R: reset | Q: quit\n";
    };

    redraw();

    while(true) {
        int key = readKey();
        if (key == 'q' || key == 'Q') break;

        if (key == 'r' || key == 'R') {
            working = grid.clone();
            for (int i = 1; i <= grid.numColors; i++) {
                paths[i].clear();
                paths[i].push_back(working.flows[i-1].start);
            }
            moves = 0;
            redraw();
            continue;
        }

        if (key >= '1' && key <= '0' + grid.numColors) {
            cur = key - '0';
            redraw();
            continue;
        }

        if (key == 'u' || key == 'U' || key == 127) {
            auto& path = paths[cur];
            if (path.size() > 1) {
                auto [r, c] = path.back();
                if (!working.cells[r][c].isEndpoint)
                    working.cells[r][c].color = 0;
                path.pop_back();
                working.flows[cur-1].path = path;
                moves++;
                redraw();
            }
            continue;
        }

        int dr = 0, dc = 0;
        if (key == 'w' || key == 'W') dr = -1;
        else if (key == 's' || key == 'S') dr = 1;
        else if (key == 'a' || key == 'A') dc = -1;
        else if (key == 'd' || key == 'D') dc = 1;
        else continue;

        auto& path = paths[cur];
        auto [cr, cc] = path.back();
        int nr = cr + dr, nc = cc + dc;

        if (nr < 0 || nr >= working.height || nc < 0 || nc >= working.width) continue;

        auto& flow = working.flows[cur - 1];
        bool target = (nr == flow.end.first && nc == flow.end.second);

        if (working.cells[nr][nc].color == 0 || target) {
            working.cells[nr][nc].color = cur;
            path.push_back({nr, nc});
            flow.path = path;
            moves++;
            
            int empty = 0;
            for  (int r = 0; r < working.height; r++)
                for (int c = 0; c < working.width; c++)
                    if (working.cells[r][c].color == 0) empty++;
            std::cerr << "empty=" << empty;
            for (auto& f: working.flows)
                std::cerr << "f" << f.color << ":back=(" << f.path.back().first << "," << f.path.back().second << ")end=(" << f.end.first << "," << f.end.second << ")";
            std::cerr << std::endl;

            if(working.isComplete()) {
                redraw();
                auto t1 = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(t1 - t0).count();
                printf("\n \033[1;32mCongratulations! Puzzle solved!\033[0m\n");
                printf(" Moves: %d | Time: %.1f seconds\n\n", moves, elapsed);
                disableRawMode();
                return;
            }

            redraw();
        }
    }
    
    disableRawMode();
    std::cout << "\n";

}

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
        enableRawMode();
        readKey();
        disableRawMode();
        playInteractive(grid);
    
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

        printf("\n\033[1mPuzzle:\033[0m %s (%dx%d, %d flows)\n\n", argv[2], grid.width, grid.height, grid.numColors);
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
            updateStats(result.timeMs, result.backtracks, result.difficulty,
                        std::max(grid.width, grid.height));
        } else {
            std::cout << "\n No solution found.\n\n";
        }

    } else if (cmd == "generate") {
        if (argc < 5) {
            std::cerr << "Usage: ./connectthedots generate <W> <H> <flows>\n";
            return 1;
        }
        int w = std::atoi(argv[2]);
        int h = std::atoi(argv[3]);
        int nf = std::atoi(argv[4]);

        printf("\nGenerating %dx%d puzzle with %d flows...\n", w, h, nf);

        Generator gen;
        auto result = gen.generate(w, h, nf);

        if (result.success) {
            Display::drawGrid(result.puzzle, "Generated Puzzle:");
            std::cout << " Difficulty: " << result.difficulty << "\n\n";

            std::string fname = "puzzles/" + std::to_string(w) + "x"
                               + std::to_string(h) + "_generated.txt";
            if (result.puzzle.saveToFile(fname))
                std::cout << " Saved to: " << fname << "\n\n";
        } else {
            std::cerr << "Failed to generate puzzle after many attempts.\n";
            return 1;
        }

    } else if (cmd == "stats") {
        showStats();

    } else {
        std::cerr << "Unknown command: " << cmd << "\n\n";
        printUsage();
        return 1;
    }

    return 0;
}