#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "wav_utils.h"

/* Signal generator process
 * Reads a WAV file and writes normalized float samples to stdout (pipe)
 * Usage: ./bin/generator <input.wav>
 */

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "[GENERATOR] Usage: " << argv[0] << " <input.wav>" << std::endl;
        return 1;
    }

    std::string input_path = argv[1];

    WavHeader header;
    std::vector<float> samples;

    if (!readWav(input_path, header, samples)) {
        return 1;
    }

    std::cerr << "[GENERATOR] Streaming " << samples.size()
              << " samples to processor..." << std::endl;

    // Write header metadata to pipe first (so processor knows sample rate etc.)
    // We send: sample_rate, num_channels, num_samples as a simple text header line
    std::cout << header.sample_rate << " "
              << header.num_channels << " "
              << samples.size() << "\n";
    std::cout.flush();

    // Stream samples as binary floats
    std::cout.write(reinterpret_cast<const char*>(samples.data()),
                    samples.size() * sizeof(float));
    std::cout.flush();

    std::cerr << "[GENERATOR] Stream complete." << std::endl;
    return 0;
}
