#include "P2PTracker.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    P2PTracker t;
    if (!t.start(6000, 6001)) {
        std::cerr << "Failed to start P2PTracker\n";
        return 1;
    }
    std::cout << "P2P tracker running. Ctrl+C to exit.\n";
    while (true) std::this_thread::sleep_for(std::chrono::seconds(60));
}