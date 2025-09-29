#include "PongDemoV2.h"
#include <iostream>

int main() {
    PongDemoV2 demo;
    if (!demo.Boot("P2P Pong Demo (Decentralized)", 1000, 1000, 1.0f,
                   "tcp://127.0.0.1:6000", "tcp://127.0.0.1:6001")) {
        std::cerr << "Failed to boot PongDemoV2\n";
        return 1;
    }
    demo.RunLoop();
    return 0;
}
