# Maze Generator

## Description

This is a simple maze game developed in C++ using SDL2 and SDL2\_image libraries.

The project uses a Depth-First Search (DFS) algorithm to generate mazes. This results in a perfect maze, meaning that there’s exactly one path between any two points, with no loops and no isolated sections.

## Settings

The game configuration is stored in a `settings.csv` file with the following format:

```
SETTING,VALUE
MAZE_WIDTH,
MAZE_HEIGHT,
CELL_SIZE,
PLAYER_SPEED,
CONST_MAZE,
PLANE_WIDTH_CM,
PLANE_HEIGHT_CM,
DISTANCE_CM,
```

* NOTE: `CONST_MAZE` is a flag represented by `0` or `1`.

## Controls

* Use the **arrow keys** to move the player through the maze.

## Maze Storage Method & Bit Representation

The maze is saved in the `export` folder both in **png** and **bin** formats.

The maze is represented in a 1D char array, where each element represents a cell in the maze. Bitfields represent whether there are walls on each side of the cell:

```
DIR_TOP = 1 << 0 (0001)

DIR_RIGHT = 1 << 1 (0010)

DIR_BOTTOM = 1 << 2 (0100)

DIR_LEFT = 1 << 3 (1000)
```

If a bit is set (1), that side of the cell is open (path), and if it’s unset (0), there is a wall. This compact representation allows efficient storage and rendering of the maze structure as well as efficient collision detection.

When generating or loading the maze, the structure is saved to or loaded from a binary file in the `export` folder for fast storage and retrieval. The binary file contains:
```
The maze width and height as two 2-byte integers.

The entire maze data as raw bytes (1 per cell).
```
## Compilation

You can build the project using one of the following methods:

### Using Makefile (recommended)

Run the following command in the project root directory:

```
make
```

This will compile the source files and create the executable `main`.

To clean build files, run:

```
make clean
```

### Manual Compilation

If you prefer to compile manually, run these commands:

```
g++ -Wall -std=c++11 -g -c src/main.cpp -o build/main.o
g++ -Wall -std=c++11 -g -c src/game.cpp -o build/game.o
g++ build/main.o build/game.o -o main -lSDL2 -lSDL2_image
```

## License

MIT
