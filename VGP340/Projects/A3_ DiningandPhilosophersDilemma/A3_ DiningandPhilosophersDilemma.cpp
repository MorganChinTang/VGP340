// A3_ DiningandPhilosophersDilemma.cpp : Dining Philosophers with std::thread and std::mutex

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class Philosopher
{
public:
    Philosopher(int id, std::vector<std::mutex>& forks, std::mutex& printMutex)
        : mId(id), mForks(forks), mPrintMutex(printMutex)
    {
        mLeftFork = id;
        mRightFork = (id + 1) % static_cast<int>(forks.size());
    }

    void DineUntil(const std::chrono::steady_clock::time_point& endTime)
    {
        while (std::chrono::steady_clock::now() < endTime)
        {
            Eat();
            std::this_thread::sleep_for(std::chrono::seconds(4));
        }
    }

    void Eat()
    {
        const int firstFork = std::min(mLeftFork, mRightFork);
        const int secondFork = std::max(mLeftFork, mRightFork);

        std::unique_lock<std::mutex> firstLock(mForks[firstFork]);
        std::unique_lock<std::mutex> secondLock(mForks[secondFork]);

        {
            std::unique_lock<std::mutex> printLock(mPrintMutex);
            std::cout << "Philosopher " << mId
                << " is eating using forks " << mLeftFork
                << " and " << mRightFork << "\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++mEatTurns;
    }

    int GetEatTurns() const
    {
        return mEatTurns;
    }

private:
    int mId = 0;
    int mLeftFork = 0;
    int mRightFork = 0;
    int mEatTurns = 0;
    std::vector<std::mutex>& mForks;
    std::mutex& mPrintMutex;
};

int main()
{
    constexpr int philosopherCount = 5;
    constexpr int runSeconds = 20;

    std::vector<std::mutex> forks(philosopherCount);
    std::mutex printMutex;

    std::vector<Philosopher> philosophers;
    philosophers.reserve(philosopherCount);
    for (int i = 0; i < philosopherCount; ++i)
    {
        philosophers.emplace_back(i, forks, printMutex);
    }

    std::vector<std::thread> threads;
    threads.reserve(philosopherCount);

    const auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(runSeconds);

    for (int i = 0; i < philosopherCount; ++i)
    {
        threads.emplace_back(&Philosopher::DineUntil, &philosophers[i], endTime);
    }

    for (std::thread& t : threads)
    {
        t.join();
    }

    std::cout << "\nSimulation Summary (" << runSeconds << " seconds):\n";
    int minTurns = philosophers[0].GetEatTurns();
    int maxTurns = philosophers[0].GetEatTurns();
    int totalTurns = 0;

    for (int i = 0; i < philosopherCount; ++i)
    {
        const int turns = philosophers[i].GetEatTurns();
        std::cout << "Philosopher " << i << " ate " << turns << " turns\n";
        minTurns = std::min(minTurns, turns);
        maxTurns = std::max(maxTurns, turns);
        totalTurns += turns;
    }

    const double averageTurns = static_cast<double>(totalTurns) / philosopherCount;
    std::cout << "Average turns: " << averageTurns << "\n";
    std::cout << "Min turns: " << minTurns << ", Max turns: " << maxTurns << "\n";

    return 0;
}
