#include "wav_utils.h"
 
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
 
bool readWav(const std::string& path, WavHeader& header, std::vector<float>& samples) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << path << std::endl;
        return false;
    }
 
    // Read the RIFF/WAVE preamble (12 bytes)
    file.read(header.riff_id,  4);
    file.read(reinterpret_cast<char*>(&header.file_size), 4);
    file.read(header.wave_id,  4);
 
    if (std::strncmp(header.riff_id, "RIFF", 4) != 0 ||
        std::strncmp(header.wave_id, "WAVE", 4) != 0) {
        std::cerr << "[ERROR] Not a valid WAV file: " << path << std::endl;
        return false;
    }
 
    // Scan chunks until we find fmt and data.
    // WAV files can contain arbitrary extra chunks (LIST, INFO, id3, etc.)
    // between the RIFF preamble and the data chunk, so we cannot assume
    // a fixed layout
    bool found_fmt  = false;
    bool found_data = false;
    uint32_t data_size = 0;
 
    while (file && !found_data) {
        char chunk_id[4];
        uint32_t chunk_size = 0;
 
        file.read(chunk_id,   4);
        file.read(reinterpret_cast<char*>(&chunk_size), 4);
 
        if (!file) break;
 
        if (std::strncmp(chunk_id, "fmt ", 4) == 0) {
            // Read the fmt fields we care about
            file.read(reinterpret_cast<char*>(&header.audio_format),    2);
            file.read(reinterpret_cast<char*>(&header.num_channels),    2);
            file.read(reinterpret_cast<char*>(&header.sample_rate),     4);
            file.read(reinterpret_cast<char*>(&header.byte_rate),       4);
            file.read(reinterpret_cast<char*>(&header.block_align),     2);
            file.read(reinterpret_cast<char*>(&header.bits_per_sample), 2);
 
            // Skip any remaining bytes in the fmt chunk (e.g. extensible format)
            if (chunk_size > 16) {
                file.seekg(chunk_size - 16, std::ios::cur);
            }
 
            if (header.audio_format != 1) {
                std::cerr << "[ERROR] Only PCM WAV files are supported." << std::endl;
                return false;
            }
 
            if (header.bits_per_sample != 16) {
                std::cerr << "[ERROR] Only 16-bit WAV files are supported." << std::endl;
                return false;
            }
 
            found_fmt = true;
 
        } else if (std::strncmp(chunk_id, "data", 4) == 0) {
            // Data chunk found — sample bytes follow immediately
            data_size  = chunk_size;
            found_data = true;
 
        } else {
            // Unknown chunk — skip it entirely
            // chunk_size may be odd; WAV pads to even byte boundary
            uint32_t skip = chunk_size + (chunk_size % 2);
            file.seekg(skip, std::ios::cur);
        }
    }
 
    if (!found_fmt) {
        std::cerr << "[ERROR] No fmt chunk found in: " << path << std::endl;
        return false;
    }
 
    if (!found_data) {
        std::cerr << "[ERROR] No data chunk found in: " << path << std::endl;
        return false;
    }
 
    // Store data_size back into header so writeWav can use it
    header.data_size = data_size;
 
    // Read raw 16-bit samples
    size_t num_samples = data_size / sizeof(int16_t);
    std::vector<int16_t> raw(num_samples);
    file.read(reinterpret_cast<char*>(raw.data()), data_size);
 
    // Normalize to [-1.0, 1.0]
    samples.resize(num_samples);
    for (size_t i = 0; i < num_samples; ++i) {
        samples[i] = static_cast<float>(raw[i]) / 32768.0f;
    }
 
    std::cerr << "[INFO] Loaded: " << path << std::endl;
    std::cerr << "[INFO] Sample rate: " << header.sample_rate << " Hz | "
              << "Channels: " << header.num_channels << " | "
              << "Samples: " << num_samples << std::endl;
 
    return true;
}
 
bool writeWav(const std::string& path, const WavHeader& header, const std::vector<float>& samples) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Cannot write file: " << path << std::endl;
        return false;
    }
 
    // Convert float back to 16-bit PCM
    std::vector<int16_t> raw(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, samples[i]));
        raw[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
 
    uint32_t data_size = static_cast<uint32_t>(raw.size() * sizeof(int16_t));
    uint32_t file_size = 36 + data_size; // 36 = size of everything after RIFF size field
 
    // Write WAV header fields explicitly to avoid struct padding issues
    // and ensure correct layout regardless of how the input was parsed.
 
    // RIFF chunk
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&file_size), 4);
    file.write("WAVE", 4);
 
    // fmt chunk
    file.write("fmt ", 4);
    uint32_t fmt_size       = 16;
    uint16_t audio_format   = 1;
    file.write(reinterpret_cast<const char*>(&fmt_size),            4);
    file.write(reinterpret_cast<const char*>(&audio_format),        2);
    file.write(reinterpret_cast<const char*>(&header.num_channels), 2);
    file.write(reinterpret_cast<const char*>(&header.sample_rate),  4);
    file.write(reinterpret_cast<const char*>(&header.byte_rate),    4);
    file.write(reinterpret_cast<const char*>(&header.block_align),  2);
    uint16_t bits = 16;
    file.write(reinterpret_cast<const char*>(&bits),                2);
 
    // data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&data_size), 4);
    file.write(reinterpret_cast<const char*>(raw.data()), data_size);
 
    std::cerr << "[INFO] Written: " << path << std::endl;
    return true;
}