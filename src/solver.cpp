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

bool Solver::isReachable(const Grid& grid, int sr, int sc, int er, int ec) {
    if (sr == er && sc == ec) return true;

    std::vector<std::vector<bool>> vis(grid.height, std::vector<bool>(grid.width, false));
    std::queue<std::pair<int,int>> q;
    q.push({sr, sc});
    vis[sr][sc] = true;

    while (!q.empty()) {
        auto [r, c] = q.front(); q.pop();
        for (auto [nr, nc] : grid.getNeighbors(r, c)) {
            if (nr == er && nc == ec) return true;
            if (!vis[nr][nc] && grid.cells[nr][nc].color == 0) {
                vis[nr][nc] = true;
                q.push({nr, nc})
            }
        }
    }
    return true;
}

bool Solver::hasIsolatedCells(const Grid& grid, const std::vector<Flow>& flows) {
    std::vector<std::vector<bool>> vis(grid.height, std::vector<bool>(grid.width, false));

    std::queue<std::pair<int,int>> q;
    for (auto& f : flows) {
        if (f.path.back() == f.end) continue;

        auto [hr, hc] = f.path.back();
        for (auto [nr, nc] : grid.getNeighbors(hr, hc)) {
            if (grid.cells[nr][nc].color == 0 && !vis[nr][nc]) {
                vis[nr][nc] = true;
                q.push({nr, nc});
            }
        }
        auto[er, ec] = f.end;
        for (auto [nr, nc] : grid.getNeighbors(er, ec)) {
            if (grid.cells[nr][nc].color == 0 && !vis[nr][nc]) {
                vis[nr][nc] = true;
                q.push({nr, nc});
            }
        }
    }

    while (!q.empty()) {
        auto [r, c] = q.front(); q.pop();
        for (auto [nr,nc] : grid.getNeighbors(r, c)) {
            if (grid.cells[nr][nc].color == 0 && !vis[nr][nc]) {
                vis[nr][nc] = true;
                q.push({nr, nc});
            }
        }
    }

    for (int r = 0; r < grid.height; r++)
        for (int c = 0; c < grid.width; c++)
            if (grid.cells[r][c].color == 0 && !vis[r][c])
                return true;
    
    return false;
}

bool Solver::allFlowsExtendable(const Grid& grid, const std::vector<Flow>& flows) {
    for (auto& f : flows) {
        if (f.path.back() == f.end) continue;
        auto [hr, hc] = f.path.back();
        bool ok = false;
        for (auto [nr, nc] : grid.getNeighbors(hr, hc)) {
            if (grid.cells[nr][nc].color == 0 || (nr == f.end.first && nc == f.end.second)) {
                ok = true;
                break;
            }
        }
        if (!ok) return false;
    }
    return true;
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
    if (done) {
        if (countEmpty(grid) != 0) return false;
        solutionCount++;
        if (solutionCount == 1) {
            firstSolution = grid;
            firstSolution.flows = flows;
        }
        return solutionCount >= maxSolutions;
    }

    if (hitLimit) return false;
    if (maxBacktracks > 0 && backtracks >= maxBacktracks) {
        hitLimit = true;
        return false;
    }

    if (!allFlowsExtendable(grid, flows)) return false;
    if (hasIsolatedCells(grid, flows)) return false;

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


        if(animate) {
            Display::clearScreen();
            Display::drawGrid(grid, "Solving...");
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(animateDelayMs));
        }
        
        bool valid = true;
        if (!atEnd) {
            for (auto& f : flows) {
                if (f.path.back() == f.end) continue;
                auto [fh_r, fh_c] = f.path.back();
                if (!isReachable(grid, fh_r, fh_c, f.end.first, f.end.second)) {
                    valid = false;
                    break;
                }
            }
        }

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
    solutionCount = 0;
    hitLimit = false;

    Grid grid = puzzle.clone();
    std::vector<Flow> flows = grid.flows;

    auto t0 = std::chrono::high_resolution_clock::now();
    solveRecursive(grid, flows);
    auto t1 = std::chrono::high_resolution_clock::now();

    result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    result.backtracks = backtracks;
    result.solutionCount = solutionCount;
    result.complete = !hitLimit;
    result.difficulty = rateDifficulty(backtracks);
    result.solved = solutionCount > 0;

    if (found) {
        result.solution = grid;
        result.solution.flows = flows;
        
    return result;
}
