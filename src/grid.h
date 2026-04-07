#pragma once 
#include <vector>
#include <utility>
#include <string>

struct Cell {
    int color = 0;
    bool isEndpoint = false;
};

struct Flow {
    int color;
    std::pair<int,int> start, end;
    std::vector<std::pair<int,int>> path;
};

class Grid {
    public: 
    int width = 0, height = 0;
    int numColors = 0;
    std::vector<std::vector<Cell>> cells;
    std::vector<Flow> flows;

    bool loadFromFile(const std::string& path);
    //todo : handle edge case where grid is 1 by 1 
    bool isComplete() const;
    bool isCellEmpty(int r, int c) const;
    std::vector<std::pair<int,int>> getNeighbors(int r, int c) const;
    Grid clone() const;
    void clear();
};