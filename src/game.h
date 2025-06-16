#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <random>
#include <vector>

#define WALL_THICKNESS 2
#define MAZE_BIN "export/maze.bin"
#define MAZE_PNG "export/maze.png"


struct Player {
    float x;
    float y;

    int cellX;
    int cellY;
};


class Game {
public:
    Game(SDL_Renderer* renderer, int width, int height, int size, bool constMaze);
    ~Game();

    void initialize();
    void render();
    void handleInput(SDL_Event& e);
    void movePlayer(float dx, float dy);

private:
    size_t index(int x, int y) const;
    bool isInMaze(int x, int y);
    void generateMaze(int x, int y);
    void saveMazeToFile(const char* filename);
    void loadMazeFromFile(const char* filename);
    bool saveTextureToFile(SDL_Renderer* renderer, 
                           SDL_Texture* texture, 
                           const char* filename, 
                           int width, int height);
    
    SDL_Renderer* renderer;
    int mazeWidth;
    int mazeHeight;
    int cellSize;
    bool constMaze;
    int windowWidth;
    int windowHeight;
    
    char* maze = nullptr;
    SDL_Texture* mazeTexture;
    
    Player player;

    std::mt19937 rng;
    
    enum Direction {
        DIR_TOP    = 1 << 0,
        DIR_RIGHT  = 1 << 1,
        DIR_BOTTOM = 1 << 2,
        DIR_LEFT   = 1 << 3
    };
    
    int dx[4] = { 0, 1, 0, -1 };
    int dy[4] = { -1, 0, 1, 0 };
    int opposite[4] = { DIR_BOTTOM, DIR_LEFT, DIR_TOP, DIR_RIGHT };
};

#endif
