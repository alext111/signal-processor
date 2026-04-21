#!/bin/bash
 
# Acoustic signal processing pipeline
# Pipes generator output directly into processor via Unix pipe (IPC)
#
# Usage: ./run_analyzer.sh <input.wav> [threshold] [window_size]
#   input.wav:   path to a 16-bit PCM mono or stereo WAV file
#   threshold:   detection threshold 0.0-1.0 (default: 0.5)
#   window_size: moving average window in samples (default: 64)
 
set -e
 
# Paths
GEN_BIN="./bin/generator"
PROC_BIN="./bin/processor"
DATA_DIR="./data"
 
# Arguments
INPUT_WAV="${1:-}"
THRESHOLD="${2:-0.5}"
WINDOW_SIZE="${3:-64}"
 
if [ -z "$INPUT_WAV" ]; then
    echo "Usage: $0 <input.wav> [threshold] [window_size]"
    exit 1
fi
 
if [ ! -f "$INPUT_WAV" ]; then
    echo "[ERROR] Input file not found: $INPUT_WAV"
    exit 1
fi
 
# Build if binaries are missing
if [ ! -f "$GEN_BIN" ] || [ ! -f "$PROC_BIN" ]; then
    echo "[INFO] Binaries not found. Building..."
    make
fi
 
# Prepare output paths
mkdir -p "$DATA_DIR"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_WAV="$DATA_DIR/filtered_$TIMESTAMP.wav"
LOG_FILE="$DATA_DIR/detections_$TIMESTAMP.txt"
 
echo "--- Sonar Analysis Session: $TIMESTAMP ---"
echo "[INFO] Input:       $INPUT_WAV"
echo "[INFO] Output WAV:  $OUTPUT_WAV"
echo "[INFO] Log file:    $LOG_FILE"
echo "[INFO] Threshold:   $THRESHOLD"
echo "[INFO] Window size: $WINDOW_SIZE samples"
echo ""
 
# IPC: pipe generator stdout directly into processor stdin
# generator writes samples → pipe → processor reads and filters
"$GEN_BIN" "$INPUT_WAV" | "$PROC_BIN" "$OUTPUT_WAV" "$THRESHOLD" "$WINDOW_SIZE" | tee "$LOG_FILE"
 
echo ""
echo "--- Session Complete ---"
echo "[INFO] Filtered audio saved to: $OUTPUT_WAV"
echo "[INFO] Detection log saved to:  $LOG_FILE"