CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
SRC_DIR  := src
BIN_DIR  := bin
 
SRCS_COMMON := $(SRC_DIR)/wav_utils.cpp
SRCS_GEN    := $(SRC_DIR)/generator.cpp
SRCS_PROC   := $(SRC_DIR)/processor.cpp
 
TARGET_GEN  := $(BIN_DIR)/generator
TARGET_PROC := $(BIN_DIR)/processor
 
OBJ_COMMON  := $(BIN_DIR)/wav_utils.o
OBJ_GEN     := $(BIN_DIR)/generator.o
OBJ_PROC    := $(BIN_DIR)/processor.o
 
.PHONY: all clean
 
all: $(BIN_DIR) $(TARGET_GEN) $(TARGET_PROC)
 
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
 
$(OBJ_COMMON): $(SRCS_COMMON) $(SRC_DIR)/wav_utils.h
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@
 
$(OBJ_GEN): $(SRCS_GEN) $(SRC_DIR)/wav_utils.h
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@
 
$(OBJ_PROC): $(SRCS_PROC) $(SRC_DIR)/wav_utils.h
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@
 
$(TARGET_GEN): $(OBJ_GEN) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@
 
$(TARGET_PROC): $(OBJ_PROC) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@
 
clean:
	rm -rf $(BIN_DIR)