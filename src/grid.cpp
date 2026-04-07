#include <fstream>
#include <map>
#include "grid.h"

bool Grid::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    file >> width >> height;
    if (width <= 0 || height <= 0) return false;

    cells.assign(height, std::vector<Cell>(width));
    std::map<char, std::vector<std::pair<int,int>>> endpoints;

    std::string token;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            file >> token;
            if (token == ".") {
                cells [r][c] = {0, false};
            } else {
                endpoints[token[0]].push_back({r, c});
                cells[r][c] = {0, true};
            }
        }
    }

    numColors = 0;
    flows.clear();
    for (auto& [ch, pts] : endpoints) {
        if (pts.size() != 2) return false;
        ++numColors;
        cells[pts[0].first][pts[0].second].color = numColors;
        cells[pts[1].first][pts[1].second].color = numColors;

        Flow f;
        f.color = numColors;
        f.start = pts[0];
        f.end = pts[1];
        f.path.push_back(pts[0]);
        flows.push_back(f);
    }

    return true;
}

bool Grid::isCellEmpty(int r, int c) const {
    if (r < 0 || r >= height || c < 0 || c >= width) return false;
    return cells[r][c].color == 0;
}

Grid Grid::clone() const {
    Grid g;
    g.width = width;
    g.height = height;
    g.numColors = numColors;
    g.cells = cells;
    g.flows = flows;
    return g;
}

std::vector<std::pair<int,int>> Grid::getNeighbors(int r, int c) const {
    std::vector<std::pair<int,int>> result;
    if (r > 0) result.push_back({r-1, c});
    if (r < height-1) result.push_back({r+1, c});
    if (c > 0) result.push_back({r, c-1});
    if (c < width-1) result.push_back({r, c+1});
    return result;
}

bool Grid::isComplete() const {
    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
            if (cells[r][c].color == 0) return false;
    for (auto& f : flows) {
        if (cells[f.end.first][f.end.second].color != f.color) return false;
        if (f.path.empty() || f.path.back() != f.end) return false;
    }
    return true;
}

void Grid::clear() {
    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
        if (!cells[r][c].isEndpoint)
            cells[r][c].color = 0;
    for (auto& f : flows) {
        f.path.clear();
        f.path.push_back(f.start);
    }
}