#include "PongDemo.h"
#include <iostream>

int main() {
    PongDemo demo;
    if (!demo.Boot("P2P Pong Demo", 1000, 1000, 1.0f,
                   "tcp://127.0.0.1:6000",
                   "tcp://127.0.0.1:6001")) {
        std::cerr << "Failed to boot PongDemo\n";
        return 1;
    }
    demo.RunLoop();
    return 0;
}
