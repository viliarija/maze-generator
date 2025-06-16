#include "game.h"
#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include <algorithm>
#include <vector>
#include <random>
#include <ctime>
#include <map>

Game::Game(SDL_Renderer* renderer, int width, int height, int size, bool constMaze)
    : renderer(renderer), mazeWidth(width), mazeHeight(height), 
      cellSize(size), constMaze(constMaze), windowWidth(width*size), 
      windowHeight(height*size), rng(std::time(nullptr)) 
{   
    // Initialize maze to 0
    maze = new char[mazeWidth * mazeHeight];
    memset(maze, 0, mazeWidth * mazeHeight * sizeof(char));
    mazeTexture = nullptr;

    // player(width * size / 2, height * size / 2),
    player.cellX = 0;
    player.cellY = 0;

    player.x = size * (player.cellX + 0.5);
    player.y = size * (player.cellY + 0.5);

    printf("%f - %f\n", player.x, player.y);
}

Game::~Game() 
{
    if (mazeTexture) {
        SDL_DestroyTexture(mazeTexture);
    }
    if (maze) {
        delete[] maze;
    }
}

bool Game::isInMaze(int x, int y) 
{
    return x >= 0 && x < mazeWidth && y >= 0 && y < mazeHeight;
}

// Find 2d object in a 1d array
size_t Game::index(int x, int y) const 
{
    return x + mazeWidth * y;
}

// algorithm using DFS
void Game::generateMaze(int x, int y) 
{
    std::vector<int> directions = { 0, 1, 2, 3 };
    std::shuffle(directions.begin(), directions.end(), rng);

    for (int dir : directions) {
        int nx = x + dx[dir];
        int ny = y + dy[dir];

        if (isInMaze(nx, ny) && maze[index(nx, ny)] == 0) {
            maze[index(x, y)] |= (1 << dir);
            maze[index(nx, ny)] |= opposite[dir];
            generateMaze(nx, ny);
        }
    }
}

void Game::initialize() 
{
    if (!(constMaze)){
        // Generate the maze from a random starting position
        int startX = rng() % mazeWidth;
        int startY = rng() % mazeHeight;
        generateMaze(startX, startY);

        saveMazeToFile(MAZE_BIN);
    } else {
        loadMazeFromFile(MAZE_BIN);
    }

    // Render maze
    mazeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, mazeWidth * cellSize, mazeHeight * cellSize);

    SDL_SetRenderTarget(renderer, mazeTexture);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Background color
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Wall color
    for (int y = 0; y < mazeHeight; ++y) {
        for (int x = 0; x < mazeWidth; ++x) {
            uint8_t cell = maze[index(x, y)];

            int px = x * cellSize;
            int py = y * cellSize;

            if (!(cell & DIR_TOP)) {
                SDL_RenderDrawLine(renderer, px, py, px + cellSize, py);
            }
            if (!(cell & DIR_RIGHT)) {
                SDL_RenderDrawLine(renderer, px + cellSize, py, px + cellSize, py + cellSize);
            }
            if (!(cell & DIR_BOTTOM)) {
                SDL_RenderDrawLine(renderer, px, py + cellSize, px + cellSize, py + cellSize);
            }
            if (!(cell & DIR_LEFT)) {
                SDL_RenderDrawLine(renderer, px, py, px, py + cellSize);
            }
        }
    }

    // Reset render target to default
    SDL_SetRenderTarget(renderer, nullptr);

    // Export maze to image
    if (!constMaze){
        if (saveTextureToFile(renderer, mazeTexture, MAZE_PNG, mazeWidth * cellSize, mazeHeight * cellSize)) {
            SDL_Log("Maze image saved successfully.");
        } else {
            SDL_Log("Failed to save maze image.");
        }
    }

}

bool Game::saveTextureToFile(SDL_Renderer* renderer, SDL_Texture* texture, const char* filename, int width, int height) {
    // Create a target texture we can render to
    SDL_Texture* target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!target) {
        SDL_Log("Failed to create target texture: %s", SDL_GetError());
        return false;
    }

    // Save the current render target and set ours
    SDL_Texture* previousTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, target);

    // Clear and render the mazeTexture onto the target
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Optional: clear background
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer); // Apply rendering

    // Read pixels from the render target
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, previousTarget);
        SDL_DestroyTexture(target);
        return false;
    }

    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) != 0) {
        SDL_Log("Failed to read pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        SDL_SetRenderTarget(renderer, previousTarget);
        SDL_DestroyTexture(target);
        return false;
    }

    // return SDL_SaveBMP(surface, filename) == 0; // For BMP
    bool success = IMG_SavePNG(surface, filename) == 0;

    // Cleanup
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, previousTarget);
    SDL_DestroyTexture(target);

    return success;
} 

void Game::saveMazeToFile(const char* filename) {
    // Open the binary file in write mode
    std::ofstream outFile(filename, std::ios::binary);

    // Check if the file is open
    if (!outFile) {
        std::cerr << "Failed to open file for saving maze." << std::endl;
        return;
    }

    // Save maze dimensions (width and height) as two 2-byte integers
    outFile.write(reinterpret_cast<const char*>(&mazeWidth), sizeof(mazeWidth));
    outFile.write(reinterpret_cast<const char*>(&mazeHeight), sizeof(mazeHeight));

    // Save maze data
    outFile.write(maze, mazeWidth * mazeHeight);

    // Close the file
    outFile.close();

    std::cout << "Maze saved to: " << filename << std::endl;
}

void Game::loadMazeFromFile(const char* filename) {
    // Open the binary file in read mode
    std::ifstream inFile(filename, std::ios::binary);

    // Check if the file is open
    if (!inFile) {
        std::cerr << "Failed to open file for loading maze." << std::endl;
        return;
    }

    // Read maze dimensions (width and height)
    inFile.read(reinterpret_cast<char*>(&mazeWidth), sizeof(mazeWidth));
    inFile.read(reinterpret_cast<char*>(&mazeHeight), sizeof(mazeHeight));

    // Read maze data into the allocated memory
    inFile.read(maze, mazeWidth * mazeHeight);

    // Close the file
    inFile.close();

    std::cout << "Maze loaded from: " << filename << std::endl;
}

// Render the game scene: maze + player
void Game::render() 
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the maze background texture
    SDL_RenderCopy(renderer, mazeTexture, NULL, NULL);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // SDL_RenderDrawPoint(renderer, (int)player.x, (int)player.y);

    int square_size = 5;

    SDL_Rect square = {
        (int)player.x - square_size / 2,
        (int)player.y - square_size / 2,
        square_size,
        square_size
    };

    SDL_RenderFillRect(renderer, &square);

    printf("[%f, %f] (%d, %d) \n", player.x, player.y, player.cellX, player.cellY);

    // Present the renderer
    SDL_RenderPresent(renderer);
}

void Game::movePlayer(float dx, float dy) 
{
    float newX = player.x + dx;
    float newY = player.y + dy;
    
    // Bounds
    if (newX <= WALL_THICKNESS) newX = WALL_THICKNESS;
    if (newY <= WALL_THICKNESS) newY = WALL_THICKNESS;
    if (newX >= (windowWidth - WALL_THICKNESS))  newX = windowWidth - WALL_THICKNESS;
    if (newY >= (windowHeight - WALL_THICKNESS)) newY = windowHeight - WALL_THICKNESS;

    // Cells
    char cell = maze[index(player.cellX, player.cellY)];

    if ((int)(newX / cellSize) < player.cellX) {
        if (!(cell & DIR_LEFT)) {
            newX = player.cellX * cellSize;
        } else { 
            player.cellX -= 1;
        }
    } else if ((int)(newX / cellSize) > player.cellX) {
        if (!(cell & DIR_RIGHT)) {
            newX = player.cellX * cellSize + (cellSize - 1);
        } else {
            player.cellX += 1;
        }
    }
    
    if ((int)(newY / cellSize) < player.cellY) {
        if (!(cell & DIR_TOP)) {
            newY = player.cellY * cellSize;
        } else { 
            player.cellY -= 1;
        }
    } else if ((int)(newY / cellSize) > player.cellY) {
        if (!(cell & DIR_BOTTOM)) {
            newY = player.cellY * cellSize + (cellSize - 1);
        } else { 
            player.cellY += 1;
        }
    }

    player.x = newX;
    player.y = newY;
}
