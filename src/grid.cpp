#include <fstream>
#include <map>
#include "grid.h"

bool Grid::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    file >> width >> height;
    if (width <= 0 || height <= 0) return false;

    cells.assign
}