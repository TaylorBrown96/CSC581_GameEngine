#pragma once
#include <functional>
#include <vector>
#include <thread>
#include "SharedData.h"

using Job = std::function<void()>;
using JobQueue = std::vector<Job>;

class JobSystem {
public:
    JobSystem(int numThreads) : numThreads(numThreads) {}
    void ExecuteJobs();  
    void ClearJobs();    
    void AddJob(const Job& job); 
    
private:
    void worker(const JobQueue& jobs);
    int numThreads;
    SharedData sharedData;
    JobQueue jobQueue; 
};
