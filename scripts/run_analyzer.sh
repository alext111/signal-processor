#!/bin/bash

# Define paths
BIN_PATH="./bin/signal_processor"
LOG_DIR="./data"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="$LOG_DIR/sonar_log_$TIMESTAMP.txt"

# Define parameters
THRESHOLD=85.0

# Use the variable later
$BIN_PATH $THRESHOLD | tee -a "$LOG_FILE"

# Ensure data directory exists
mkdir -p "$LOG_DIR"

# Check if the binary exists, if not, build it
if [ ! -f "$BIN_PATH" ]; then
    echo "Binary not found. Compiling..."
    make
fi

# Run the program and redirect output to both console and log file
echo "--- Starting Sonar Session: $TIMESTAMP ---" | tee -a "$LOG_FILE"
$BIN_PATH $THRESHOLD | tee -a "$LOG_FILE"
echo "--- Session Complete. Log saved to $LOG_FILE ---"