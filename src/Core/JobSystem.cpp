#include "JobSystem.h"
#include <cstddef>

void JobSystem::worker(const JobQueue& jobs) {
    while (true) {
        size_t jobIndex = sharedData.nextJobIndex.fetch_add(1);
        if (jobIndex >= jobs.size()) {
            break;
        }
        jobs[jobIndex]();
    }
}

void JobSystem::ExecuteJobs() {
    if (jobQueue.empty()) {
        return;
    }
    
    // Reset job index
    sharedData.nextJobIndex = 0;
    
    // Create worker threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(&JobSystem::worker, this, std::ref(jobQueue));
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
}

void JobSystem::ClearJobs() {
    jobQueue.clear();
}

void JobSystem::AddJob(const Job& job) {
    jobQueue.push_back(job);
}
