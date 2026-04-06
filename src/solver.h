#pragma once 
#include "grid.h"

struct SolveResult {
    bool solved = false;
    bool complete = true;
    Grid solution;
    int backtracks = 0;
    int solutionCount = 0;
    double timeMs = 0;
    std::string difficulty;
};

class Solver {
    public:
        bool animate = false;
        int animateDelayMs = 75;

        SolveResult solve(const Grid& puzzle);

    private:
        int backtracks = 0;

        bool solveRecursive(Grid& grid, std::vector<Flow>& flows);
        int pickNextFlow(const Grid& grid, const std::vector<Flow>& flows);
        int countEmpty(const Grid& grid);
        std::string rateDifficulty(int backtracks);
};