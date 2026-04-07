#pragma once
#include "grid.h"
#include <string>

struct GenerateResult {
    bool success = false;
    Grid puzzle;
    std::string difficulty;
};

class Generator {
    public:
        GenerateResult generate(int width, int height, int numFlows);
    
    private: 
        bool fillGrid(Grid& grid, std::vector<Flow>& flows, int idx, int total);
        bool randomWalk(Grid& grid, Flow& flow, int minLen);
};