#include <random>
#include <algorithm>
#include <iostream>
#include "generator.h"
#include "solver.h"

static std::mt19937 rng(std::random_device{}());

bool Generator::randomWalk