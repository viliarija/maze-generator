#include "game.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>
#include <cmath>

const int TARGET_FPS = 60;
const double TARGET_FRAME_TIME = 1000.0 / TARGET_FPS;

float PLANE_WIDTH_CM;
float PLANE_HEIGHT_CM;
float DISTANCE_CM;

float PX_PER_CM_X;
float PX_PER_CM_Y;
float CENTER_X_PX;
float CENTER_Y_PX;

std::map<std::string, std::string> readSettingsCSV(const std::string& filename) {
    std::map<std::string, std::string> settings;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open settings file.\n";
        return settings;
    }

    std::string key, value, line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        size_t comma = line.find(',');
        if (comma != std::string::npos) {
            key = line.substr(0, comma);
            value = line.substr(comma + 1);
            settings[key] = value;
        }
    }

    return settings;
}

inline float degrees(float radians) {
    return radians * 180.0f / M_PI;
}

void calculateRayAngles(float x_px, float y_px, float& angleX_deg, float& angleY_deg) {
    // Pixel offset from center
    float dx_px = x_px - CENTER_X_PX;
    float dy_px = y_px - CENTER_Y_PX;

    // Convert to cm offset
    float dx_cm = dx_px / PX_PER_CM_X;
    float dy_cm = -dy_px / PX_PER_CM_Y; // Flip Y axis

    // Calculate angles
    angleX_deg = degrees(atan2(dx_cm, DISTANCE_CM));
    angleY_deg = degrees(atan2(dy_cm, DISTANCE_CM));
}

int main() {

    // Take in the settings
    auto settings = readSettingsCSV("settings.csv");

    int mazeWidth = std::stoi(settings["MAZE_WIDTH"]);
    int mazeHeight = std::stoi(settings["MAZE_HEIGHT"]);
    int cellSize = std::stoi(settings["CELL_SIZE"]);
    float speed = std::stof(settings["PLAYER_SPEED"]);
    bool constMaze = std::stof(settings["CONST_MAZE"]);

    // Math constants
    float PLANE_WIDTH_CM = std::stoi(settings["PLANE_WIDTH_CM"]);
    float PLANE_HEIGHT_CM = std::stoi(settings["PLANE_HEIGHT_CM"]);
    float DISTANCE_CM = std::stoi(settings["DISTANCE_CM"]);

    float PX_PER_CM_X = (float)(mazeWidth * cellSize) / PLANE_WIDTH_CM;
    float PX_PER_CM_Y = (float)(mazeHeight * cellSize) / PLANE_HEIGHT_CM;
    float CENTER_X_PX = (float)(mazeWidth * cellSize) / 2.0f;
    float CENTER_Y_PX = (float)(mazeHeight * cellSize) / 2.0f;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Maze Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mazeWidth*cellSize, mazeHeight*cellSize, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer) {
        std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Initialize the game object
    Game game(renderer, mazeWidth, mazeHeight, cellSize, constMaze);
    game.initialize();

    // Main loop flag
    bool quit = false;
    SDL_Event e;
    
    auto targetFrameTime = std::chrono::duration<double, std::milli>(TARGET_FRAME_TIME);

    bool keys[4] = {false, false, false, false}; // up, down, left, right

    while (!quit) {
        auto frameStartTime = std::chrono::high_resolution_clock::now();

        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        keys[0] = true;
                        break;
                    case SDLK_DOWN:
                        keys[1] = true;
                        break;
                    case SDLK_LEFT:
                        keys[2] = true;
                        break;
                    case SDLK_RIGHT:
                        keys[3] = true;
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        keys[0] = false;
                        break;
                    case SDLK_DOWN:
                        keys[1] = false;
                        break;
                    case SDLK_LEFT:
                        keys[2] = false;
                        break;
                    case SDLK_RIGHT:
                        keys[3] = false;
                        break;
                }
            }
        }
        
        // Update player position
        if (keys[0]) {
            game.movePlayer(0, -speed);
        }
        if (keys[1]) {
            game.movePlayer(0, speed);
        }
        if (keys[2]) {
            game.movePlayer(-speed, 0);
        }
        if (keys[3]) {
            game.movePlayer(speed, 0);
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the game
        game.render();
        SDL_RenderPresent(renderer);

        // Limmit frame rate
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> frameDuration = frameEndTime - frameStartTime;

        if (frameDuration < targetFrameTime) {
            std::chrono::duration<double, std::milli> sleepTime = targetFrameTime - frameDuration;
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime.count())));
        }
    }

    // Clean up SDL and other resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}