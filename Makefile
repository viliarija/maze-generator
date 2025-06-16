# Compiler settings
CXX = g++
CXXFLAGS = -Wall -std=c++11 -g
LDFLAGS = -lSDL2 -lSDL2_image

# Directories
SRC_DIR = src
BUILD_DIR = build

# Files
SRC_FILES = $(SRC_DIR)/main.cpp $(SRC_DIR)/game.cpp
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executable name
EXEC = main

# Default target
all: $(EXEC)

# Create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile .cpp files to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link the object files to create the executable
$(EXEC): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(EXEC) $(LDFLAGS)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: all clean
