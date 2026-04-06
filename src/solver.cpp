#include "solver.h"
#include "display.h"
#include <iostream>
#include <algorithm>
#include <queue>
#include <chrono>
#include <thread>

std::string Solver::rateDifficulty(int bt) {
    if (bt < 50) return "Easy";
    if (bt < 500) return "Medium";
    if (bt < 5000) return "Hard";
    return "Expert";
}

int Solver::countEmpty(const Grid& grid) {
    int n = 0;
    for (int r = 0; r < grid.height; r++)
        for (int c = 0; c < grid.width; c++)
            if (grid.cells[r][c].color == 0) n++;
    return n;
}

int Solver::pickNextFlow(const Grid& grid, const std::vector<Flow>& flows) {
    int best = -1, bestN = 999999;
    for (int i = 0; i < (int)flows.size(); i++) {
        if (flows[i].path.back() == flows[i].end) continue;
        auto [hr, hc] = flows[i].path.back();
        int n = 0;
        for (auto [nr, nc] : grid.getNeighbors(hr, hc)) {
            if (grid.cells[nr][nc].color == 0 || 
               (nr == flows[i].end.first && nc == flows[i].end.second))
                n++;
            }
        if (n < bestN) {
            bestN = n;
            best = i;
        }
    }
    if (best < 0)
        std::cerr << "weird no incomplete flow found\n";
    return best;
}

bool Solver::solveRecursive(Grid& grid, std::vector<Flow>& flows) {
    bool done = true;
    for (auto& f : flows) {
        if (f.path.back() != f.end) { done = false; break; }
    }
    if (done) return countEmpty(grid) == 0;

    int fi = pickNextFlow(grid, flows);
    if (fi < 0) return false;

    Flow& flow = flows[fi];
    auto [hr, hc] = flow.path.back();

    std::vector<std::pair<int,int>> moves;
    for (auto [nr,nc] : grid.getNeighbors(hr, hc)) {
        if (grid.cells[nr][nc].color == 0) {
            moves.push_back({nr, nc});
        } else if (nr == flow.end.first && nc == flow.end.second && grid.cells[nr][nc].color == flow.color) {
            moves.push_back({nr, nc});
        }
    }

    int er = flow.end.first, ec = flow.end.second;
    std::sort(moves.begin(), moves.end(), [er, ec](auto& a, auto& b) {
        int da = std::abs(a.first - er) + std::abs(a.second - ec);
        int db = std::abs(b.first - er) + std::abs(b.second - ec);
        return da < db;
    });

    for (auto [nr, nc] : moves) {
        bool atEnd = (nr == flow.end.first && nc == flow.end.second);

        int prev = grid.cells[nr][nc].color;
        grid.cells[nr][nc].color = flow.color;
        flow.path.push_back({nr, nc});

        if (solveRecursive(grid,flows))
            return true;
        
        backtracks++;
        flow.path.pop_back();
        grid.cells[nr][nc].color = prev;
    }

    return false;
}

SolveResult Solver::solve(const Grid& puzzle) {
    SolveResult result;
    backtracks = 0;

    Grid grid = puzzle.clone();
    std::vector<Flow> flows = grid.flows;

    auto t0 = std::chrono::high_resolution_clock::now();
    bool found = solveRecursive(grid, flows);
    auto t1 = std::chrono::high_resolution_clock::now();

    result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    result.backtracks = backtracks;
    result.difficulty = rateDifficulty(backtracks);
    result.solved = found;
    if (found) {
        result.solution = grid;
        result.solution.flows = flows;
    }

    return result;
}
