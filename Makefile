# Variables
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = bin/signal_processor
SRC = src/main.cpp

# Default target: builds the program
all: setup
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Create the binary directory if it doesn't exist
setup:
	mkdir -p bin

# Clean up build artifacts
clean:
	rm -rf bin/

# A "phony" target just for testing the build
test: all
	./$(TARGET) TEST_MODE