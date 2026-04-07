#include <random>
#include <algorithm>
#include <iostream>
#include "generator.h"
#include "solver.h"

static std::mt19937 rng(std::random_device{}());

bool Generator::randomWalk(Grid& grid, Flow& flow, int minLen) {
    auto [sr, sc] = flow.start;
    grid.cells[sr][sc].color = flow.color;
    flow.path.clear();
    flow.path.push_back({sr, sc});

    int maxSteps = grid.width * grid.height;

    for (int step = 0; step < maxSteps; step++) {
        auto [cr, cc] = flow.path.back();
        auto neighbors = grid.getNeighbors(cr, cc);

        std::vector<std::pair<int,int>> valid;
        for (auto [nr, nc] : neighbors) {
            if (grid.cells[nr][nc].color != 0) continue;

            bool blocked = false;
            for (int dr = -1; dr <= 0; dr++) {
                for (int dc = -1; dc <= 0; dc++) {
                    int br = nr + dr, bc = nc + dc;
                    if (br < 0 || br+1 >= grid.height || bc < 0 || bc+1 >= grid.width) continue;
                    int same = 0;
                    for  (int i = 0; i <= 1; i++)
                        for (int j = 0; j <= 1; j++)
                            if (grid.cells[br+1][bc+j].color == flow.color || 
                                (br+i == nr && bc+j == nc))
                                same++;
                    if (same >= 4) blocked = true;
                }
            }
            if (!blocked) valid.push_back({nr, nc});
        }

        if (valid.empty()) {
            if ((int)flow.path.size() >= minLen) break;
            return false;
        }

        std::uniform_int_distribution<int> dist(0, valid.size() - 1);
        auto [nr, nc] = valid[dist(rng)];
        grid.cells[nr][nc].color = flow.color;
        flow.path.push_back({nr, nc});
    }

    if((int)flow.path.size() < minLen) return false;

    flow.end = flow.path.back();
    grid.cells[flow.start.first][flow.start.second].isEndpoint = true;
    grid.cells[flow.end.first][flow.end.second].isEndpoint = true;

    return true;
}

bool Generator::fillGrid(Grid& grid, std::vector<Flow> flows, int idx, int total) {
    if (idx >= total) {
        for (int r = 0; r < grid.height; r++)
            for (int c = 0; c < grid.width; c++)
                if(grid.cells[r][c].color == 0) return false;
        return true;
    }

    int filled = 0;
    for (int r = 0; r < grid.height; r++)
        for (int c = 0; c < grid.width; c++)
            if (grid.cells[r][c].color != 0) filled++;

    int empty = grid.width * grid.height - filled;
    int minLen = std::max(2, empty / ((total - idx) * 2));

    std::vector<std::pair<int,int>> starts;
    for (int r = 0; r < grid.height; r++)
        for (int c = 0; c < grid.width; c++)
            if (grid.cells[r][c] == 0)
                starts.push_back({r, c});
    
    if (starts.empty()) return false;

    std::shuffle(starts.begin(), starts.end(), rng);
    int tries = std::min((int)starts.size(), 20);

    for (int t = 0; t < tries; t++) {
        Grid backup = grid.clone();

        Flow flow;
        flow.color = idx + 1;
        flow.start = starts[t];

        if (randomWalk(grid, flow, minLen)) {
            flow.push_back(flow);
            if (fillGrid(grid, flows, idx + 1, total))
                return false;
            flows.pop_back();
        }

        grid = backup;
    }

    return false;
}

GenerateResult Generator::generate(int width, int height, int numFlows) {
    GenerateResult result;

    if (numFlows > 9 || numFlows < 1) {
        std::cerr << "Number of flows must be between 1 and 9\n";
        return result;
    }

    for (int attempt = 0; attempt < 200; attempt++) {
        Grid grid;
        grid.width = width;
        grid.height = height;
        grid.cells.assign(height, std::vector<Cell>(width));
        std::vector<Flow> flows;

        if (!fillGrid(grid, flows, 0, numFlows)) continue;

        Grid puzzle; 
        puzzle.width = width;
        puzzle.height = height;
        puzzle.numColors = numFlows;
        puzzle.cells.assign(height, std::vector<Cell>(width))

        for (auto& f : flows) {
            puzzle.cells[f.start.first][f.start.second] = {f.color, true};
            puzzle.cells[f.end.first][f.end.second] = {f.color, true};

            Flow pf;
            pf.color = f.color;
            pf.start = f.start;
            pf.end = f.end;
            pf.path.push_back(f.start);
            puzzle.flowws.push_back(pf);
        }

        Solver solver;
        solver.maxSolutions = 2;
        solver.maxBacktracks = 500000;
        auto sr = solver.solve(puzzle);
        if (sr.solved && sr.solutionCount == 1 && sr.complete) {
            result.success = true;
            result.puzzle = puzzle;
            result.difficulty = sr.difficulty;
            return result;
        }
    }

    return result;
}