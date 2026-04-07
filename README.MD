# Connect the dots

a terminal flow puzzle game and solver. place paths between colored dot pairs so every cell is filled and no paths cross. like flow free but in the terminal. those basic FaceBook games we used to play on our mother's phones

built this to learn backtracking with constraint propagation. the solver uses BFS reachability pruning and isolated cell detection to cut the search space. my teacher at school taught this in class so i decided it would be good to practice here.

## build & run 
```bash
make
./connectthedots play puzzles/5x5_easy.txt
./connectthedots solve puzzles/7x7_hard.txt --animate
./connectthedots generate 6 6 4
./connectthedots stats
```

## controls (da play mode)

- WASD or arrows to move around
- 1-9 to switch flow color 
- U to undo
- R to reset 
- Q to quit

## how the solver works

recursie backtracking with three pruning strategies:
- check every incomplete flow can still reach its endpoint
- check no empty cells are completely isolated
- check every flow head has at least one valid move

## built with 

- c++17, no external lib
- ANSI escape codes for colors
- box drawing unicode characters for the grid 