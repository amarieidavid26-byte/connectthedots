#include <stringe>
#include "grid.h"

namespace Display {
    std::string colorCode(int color);
    std::string resetCode();
    std::string boldCode();
    void drawGrid(const Grid& grid);
    void drawGrid(const Grid& grid, const std::string& title);
    void clearScreen();
    std::string pathChar(const Grid& grid, int r, int c);
}