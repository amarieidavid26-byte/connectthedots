#include "display.h"
#include <iostream>

namespace Display {

std::string colorCode(int color) {
    switch (color) {
        case 1: return "\033[31m";
        case 2: return "\033[32m";
        case 3: return "\033[33m";
        case 4: return "\033[34m";
        case 5: return "\033[35m";
        case 6: return "\033[36m";
        case 7: return "\033[37m";
        case 8: return "\033[91m";
        case 9: return "\033[92m";
        default: return "\033[90m";
    }
}

std::string resetCode() { return "\033[0m"; }
std::string boldCode() { return "\033[1m"; }

static bool sameColor(const Grid& grid, int r, int c, int color) {
    if (r < 0 || r >= grid.height || c < 0 || c >= grid.width) return false;
    return grid.cells[r][c].color == color;
}

void clearScreen() {
    std::cout << "\033[2J\033[H";
}

std::string pathChar(const Grid& grid, int r, int c) {
    int col = grid.cells[r][c].color;
    if (col == 0) return "\u00B7";
    if (grid.cells[r][c].isEndpoint) return "\u25CF";

    bool up = sameColor(grid, r-1, c, col);
    bool down = sameColor(grid, r+1, c, col);
    bool left = sameColor(grid, r, c-1, col);
    bool right = sameColor(grid, r, c+1, col);

    if(up && down && !left && !right) return "\u2503";
    if(!up && !down && left && right) return "\u2501";
    if(!up && down && !left && right) return "\u250F";
    if(!up && down && left && !right) return "\u2513";
    if(up && !down && !left && right) return "\u2517";
    if(up && !down && left && !right) return "\u251B";

    if(up || down) return "\u2503";
    if(left || right) return "\u2501";
    return "\u25AA";
    
}

void drawGrid(const Grid& grid) { drawGrid(grid, ""); }

void drawGrid(const Grid& grid, const std::string& title) {
    if (!title.empty())
        std::cout << "\033[1m" << title << resetCode() << "\n";

    std::cout << " \u250C";
    for (int c = 0; c < grid.width; c++) {
        std::cout << "\u2500\u2500";
        if (c < grid.width - 1) std::cout << "\u2500";
    }
    std::cout << "\u2500\u2510\n";

    for (int r = 0; r < grid.height; r++) {
        std::cout << "\u2502";
        for (int c = 0; c < grid.width; c++) {
            int col = grid.cells[r][c].color;
            std::string ch = pathChar(grid, r, c);

            if(col == 0) {
                std::cout << colorCode(0) << " " << ch << resetCode();
            } else if (grid.cells[r][c].isEndpoint) {
                std::cout << " " << colorCode(col) << boldCode() << ch << resetCode();
            } else {
                std::cout << " " << colorCode(col) << ch << resetCode();
            }
        }
        std::cout << " \u2502\n";
    }

    std::cout << " \u2514";
    for (int k = 0; k < grid.width; k++) {
        std::cout << "\u2500\u2500";
        if (k < grid.width - 1) std::cout << "\u2500";
    }
    std::cout << "\u2500\u2518\n";

    std::cout << " Colors: ";
    for (int i = 1; i <= grid.numColors; i++) {
        std::cout << colorCode(i) << boldCode() << (char)('A' + i - 1)
                  << resetCode() << " ";
    }
    std::cout << "\n";
}

}