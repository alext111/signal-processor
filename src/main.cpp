#include <iostream>
#include <vector>
#include <cstdlib>   // For rand()
#include <ctime>     // For time()

int main(int argc, char* argv[]) {
    // Setup Parameters
    // If a number is passed via Bash, use it as the threshold. Otherwise, default to 70.
    float threshold = (argc > 1) ? std::atof(argv[1]) : 70.0f;
    
    std::cout << "[SYSTEM] Starting Sonar Scan. Threshold: " << threshold << std::endl;

    // Seed the random number generator
    std::srand(std::time(0));

    // Simulate 50 "data packets" from the sensor
    for (int i = 0; i < 50; ++i) {
        // Generate noise between 0 and 100
        float signalStrength = static_cast<float>(std::rand() % 100);

        // Detection logic
        if (signalStrength > threshold) {
            std::cout << "[DETECTION] Spike Found! Index: " << i 
                      << " | Strength: " << signalStrength << " dB" << std::endl;
        }
    }

    std::cout << "[SYSTEM] Scan Complete." << std::endl;
    return 0;
}