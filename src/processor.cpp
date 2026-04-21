#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>
#include "wav_utils.h"

/* Signal processor process
 * Reads samples from stdin (pipe), applies moving average filter,
 * detects threshold crossings, and writes filtered WAV output.

 * Usage: ./bin/processor <output.wav> [threshold] [window_size]
 * threshold:   detection threshold (default 0.5, range 0.0-1.0)
 * window_size: moving average window in samples (default 64)

 * Apply a simple moving average (low-pass) filter.
 * Each output sample is the mean of the surrounding window_size samples.
 * This attenuates high-frequency noise while preserving lower-frequency signal.
 */
std::vector<float> movingAverageFilter(const std::vector<float>& input, int window_size) {
    std::vector<float> output(input.size(), 0.0f);
    int half = window_size / 2;

    for (size_t i = 0; i < input.size(); ++i) {
        float sum = 0.0f;
        int count = 0;
        for (int j = -half; j <= half; ++j) {
            int idx = static_cast<int>(i) + j;
            if (idx >= 0 && idx < static_cast<int>(input.size())) {
                sum += input[idx];
                count++;
            }
        }
        output[i] = sum / static_cast<float>(count);
    }

    return output;
}

/* Detect threshold crossings in the filtered signal.
 * A detection is a point where the absolute value exceeds the threshold.
 * Consecutive detections within min_gap samples are merged into one event
 * to avoid reporting the same pulse multiple times.
 */
struct Detection {
    size_t   sample_index;
    float    strength;
    double   timestamp_ms;
};

std::vector<Detection> detectPulses(
    const std::vector<float>& signal,
    float threshold,
    uint32_t sample_rate,
    size_t min_gap = 1000)
{
    std::vector<Detection> detections;
    size_t last_detection = 0;

    for (size_t i = 0; i < signal.size(); ++i) {
        float abs_val = std::fabs(signal[i]);
        if (abs_val > threshold) {
            // Enforce minimum gap between detections
            if (detections.empty() || (i - last_detection) >= min_gap) {
                double timestamp_ms = (static_cast<double>(i) / sample_rate) * 1000.0;
                detections.push_back({ i, abs_val, timestamp_ms });
                last_detection = i;
            }
        }
    }

    return detections;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "[PROCESSOR] Usage: " << argv[0]
                  << " <output.wav> [threshold] [window_size]" << std::endl;
        return 1;
    }

    std::string output_path = argv[1];
    float threshold   = (argc > 2) ? std::atof(argv[2]) : 0.5f;
    int   window_size = (argc > 3) ? std::atoi(argv[3]) : 64;

    std::cerr << "[PROCESSOR] Starting. Threshold: " << threshold
              << " | Window: " << window_size << " samples" << std::endl;

    // Read metadata line from pipe
    uint32_t sample_rate, num_channels;
    size_t   num_samples;

    std::string meta_line;
    std::getline(std::cin, meta_line);
    std::istringstream meta_ss(meta_line);
    meta_ss >> sample_rate >> num_channels >> num_samples;

    std::cerr << "[PROCESSOR] Receiving: " << num_samples
              << " samples @ " << sample_rate << " Hz" << std::endl;

    // Read binary float samples from pipe
    std::vector<float> samples(num_samples);
    std::cin.read(reinterpret_cast<char*>(samples.data()),
                  num_samples * sizeof(float));

    size_t received = std::cin.gcount() / sizeof(float);
    if (received != num_samples) {
        std::cerr << "[PROCESSOR] Warning: expected " << num_samples
                  << " samples, received " << received << std::endl;
        samples.resize(received);
    }

    std::cerr << "[PROCESSOR] Applying moving average filter (window="
              << window_size << ")..." << std::endl;

    // Apply low-pass filter
    std::vector<float> filtered = movingAverageFilter(samples, window_size);

    // Detect pulses
    std::cerr << "[PROCESSOR] Running pulse detection..." << std::endl;
    std::vector<Detection> detections = detectPulses(filtered, threshold, sample_rate);

    // Report detections to stdout (visible in terminal / log file)
    std::cout << "[PROCESSOR] Detection report" << std::endl;
    std::cout << "[PROCESSOR] Threshold: " << threshold
              << " | Window: " << window_size
              << " | Sample rate: " << sample_rate << " Hz" << std::endl;
    std::cout << "[PROCESSOR] Total detections: " << detections.size() << std::endl;

    for (const auto& d : detections) {
        std::cout << "[DETECTION] Sample: " << d.sample_index
                  << " | Time: "    << d.timestamp_ms << " ms"
                  << " | Strength: " << d.strength << std::endl;
    }

    // Write filtered output WAV
    WavHeader out_header;
    std::memcpy(out_header.riff_id,  "RIFF", 4);
    std::memcpy(out_header.wave_id,  "WAVE", 4);
    std::memcpy(out_header.fmt_id,   "fmt ", 4);
    std::memcpy(out_header.data_id,  "data", 4);
    out_header.fmt_size        = 16;
    out_header.audio_format    = 1;
    out_header.num_channels    = static_cast<uint16_t>(num_channels);
    out_header.sample_rate     = sample_rate;
    out_header.bits_per_sample = 16;
    out_header.block_align     = num_channels * 2;
    out_header.byte_rate       = sample_rate * num_channels * 2;
    out_header.data_size       = static_cast<uint32_t>(filtered.size() * sizeof(int16_t));
    out_header.file_size       = sizeof(WavHeader) - 8 + out_header.data_size;

    if (!writeWav(output_path, out_header, filtered)) {
        return 1;
    }

    std::cerr << "[PROCESSOR] Complete." << std::endl;
    return 0;
}
