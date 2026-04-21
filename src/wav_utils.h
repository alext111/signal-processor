#pragma once

#include <cstdint>
#include <string>
#include <vector>

// WAV file header (PCM format)
struct WavHeader {
    // RIFF chunk
    char     riff_id[4];       // "RIFF"
    uint32_t file_size;        // Total file size - 8
    char     wave_id[4];       // "WAVE"

    // fmt chunk
    char     fmt_id[4];        // "fmt "
    uint32_t fmt_size;         // 16 for PCM
    uint16_t audio_format;     // 1 = PCM
    uint16_t num_channels;     // 1 = mono, 2 = stereo
    uint32_t sample_rate;      // e.g. 44100
    uint32_t byte_rate;        // sample_rate * num_channels * bits/8
    uint16_t block_align;      // num_channels * bits/8
    uint16_t bits_per_sample;  // 8, 16, etc.

    // data chunk
    char     data_id[4];       // "data"
    uint32_t data_size;        // Number of bytes of sample data
};

// Read a 16-bit PCM WAV file into a vector of floats (normalized -1.0 to 1.0)
bool readWav(const std::string& path, WavHeader& header, std::vector<float>& samples);

// Write a vector of floats back to a 16-bit PCM WAV file
bool writeWav(const std::string& path, const WavHeader& header, const std::vector<float>& samples);
