# Acoustic Signal Processor

A C++17 command-line pipeline that reads a WAV audio file, filters noise, detects acoustic pulses, and writes a cleaned output file alongside a timestamped detection log.

Tested against sonar recordings sourced from [Freesound.org](https://freesound.org).

## Overview

The pipeline is split into two separate programs that run simultaneously and communicate through a Unix pipe. The first program reads the audio file and streams its samples to the second, which applies the filter, runs detection, and writes the output. This separation keeps the data acquisition and processing stages independent.

```
generator  ──── Unix pipe ────►  processor
(reads WAV,                      (filters samples,
 streams samples)                 detects pulses,
                                  writes output)
```

## How it works

**Reading the audio file**

The generator reads a 16-bit PCM WAV file and converts the raw audio samples into normalized float values between -1.0 and 1.0. This normalized range makes the threshold settings consistent regardless of the original file's recording level. The sample data is then streamed to the processor over a Unix pipe.

**Noise filtering**

The processor applies a moving average filter to the incoming samples before running detection. Each sample is replaced with the average of its surrounding neighbors, which smooths out short bursts of noise while preserving the shape of longer acoustic events. The number of neighbors considered is controlled by the window size parameter.

**Pulse detection**

After filtering, the processor scans the signal for samples whose amplitude exceeds the detection threshold. A minimum gap between detections prevents a single loud event from being counted multiple times. Each detection is recorded with its sample position, timestamp in milliseconds, and signal strength.

**Output**

The filtered audio is written to a new WAV file. Detection events are printed to the terminal and saved to a log file.

## Build

Requires `g++` and `make`. No external dependencies.

```bash
make
```

Produces two binaries in `bin/`:
- `bin/generator` — reads and streams the WAV file
- `bin/processor` — filters, detects, and writes output

```bash
make clean
```

## Usage

```bash
./run_analyzer.sh <input.wav> [threshold] [window_size]
```

| Argument      | Description                          | Default |
|---------------|--------------------------------------|---------|
| `input.wav`   | 16-bit PCM WAV file (mono or stereo) | —       |
| `threshold`   | Detection sensitivity (0.0 – 1.0)    | `0.5`   |
| `window_size` | Filter strength in samples           | `64`    |

A lower threshold catches quieter events but may produce more false detections. A larger window applies stronger filtering but can muffle detail in the audio output.

### Example

```bash
./run_analyzer.sh samples/audio_file.wav 0.6 16
```

Output files are written to `data/` with a timestamp:
- `data/filtered_<timestamp>.wav` — filtered audio
- `data/detections_<timestamp>.txt` — detection log

### Example detection log

```
[PROCESSOR] Detection report
[PROCESSOR] Threshold: 0.6 | Window: 16 | Sample rate: 44100 Hz
[PROCESSOR] Total detections: 4
[DETECTION] Sample: 12480  | Time: 283.04 ms  | Strength: 0.821
[DETECTION] Sample: 38910  | Time: 882.54 ms  | Strength: 0.763
[DETECTION] Sample: 71220  | Time: 1615.19 ms | Strength: 0.914
[DETECTION] Sample: 98340  | Time: 2229.93 ms | Strength: 0.708
```

## WAV file requirements

| Property    | Requirement             |
|-------------|-------------------------|
| Format      | PCM (uncompressed)      |
| Bit depth   | 16-bit                  |
| Channels    | Mono or stereo          |
| Sample rate | Any (44100 recommended) |

## Project structure

```
signal-processor/
├── src/
│   ├── generator.cpp       Reads WAV file and streams samples to processor
│   ├── processor.cpp       Applies filter, runs detection, writes output
│   ├── wav_utils.h         WAV file structure and function declarations
│   └── wav_utils.cpp       WAV file reading and writing implementation
├── tests/                  Unit tests (in progress)
├── samples/                Input WAV files for testing (gitignored)
├── data/                   Output audio and detection logs (gitignored)
├── bin/                    Compiled binaries (gitignored)
├── Makefile
├── run_analyzer.sh         Builds if needed, runs the pipeline, saves output
└── README.md
```
