#include "P2PTrackerV2.h"
#include <iostream>
#include <chrono>
#include <thread>

int main(){
    P2PTrackerV2 t;
    if (!t.start(6000,6001)) { std::cerr << "Failed to start P2PTrackerV2\n"; return 1; }
    std::cout << "P2PTrackerV2 running. Ctrl+C to exit.\n";
    while(true) std::this_thread::sleep_for(std::chrono::seconds(60));
}
