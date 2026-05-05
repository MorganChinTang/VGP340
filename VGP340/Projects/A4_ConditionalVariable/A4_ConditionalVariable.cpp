// A4_ConditionalVariable.cpp : Particle stream with circular buffer and async quadrant consumers

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

struct Point2D
{
    float x = 0.0f;
    float y = 0.0f;
};

int GetQuadrant(const Point2D& point)
{
    if (point.x >= 0.0f && point.y >= 0.0f)
        return 1;
    if (point.x < 0.0f && point.y >= 0.0f)
        return 2;
    if (point.x < 0.0f && point.y < 0.0f)
        return 3;
    return 4;
}

double Distance(const Point2D& a, const Point2D& b)
{
    const double dx = static_cast<double>(a.x) - static_cast<double>(b.x);
    const double dy = static_cast<double>(a.y) - static_cast<double>(b.y);
    return std::sqrt(dx * dx + dy * dy);
}

std::atomic_bool done = false;

struct CircularBuffer
{
    Point2D* buffer;
    int capacity;

    int frontIndex;
    int rearIndex;
    int count;
    std::mutex mutex;
    std::condition_variable notEmpty;
    std::condition_variable notFull;

    CircularBuffer(int cap)
        : capacity(cap)
        , frontIndex(0)
        , rearIndex(0)
        , count(0)
        , buffer(new Point2D[cap])
    {
    }

    ~CircularBuffer()
    {
        delete[] buffer;
    }

    void Push(const Point2D& point)
    {
        std::unique_lock<std::mutex> lk(mutex);
        notFull.wait(lk, [this]() { return count != capacity; });

        buffer[rearIndex] = point;
        rearIndex = (rearIndex + 1) % capacity;
        ++count;

        lk.unlock();
        notEmpty.notify_all();
    }

    bool PopForQuadrant(int quadrant, Point2D& point)
    {
        std::unique_lock<std::mutex> lk(mutex);
        notEmpty.wait(lk, [this, quadrant]() {
            return done.load() || HasPointForQuadrant(quadrant);
            });

        if (!HasPointForQuadrant(quadrant))
        {
            return false;
        }

        int matchOffset = -1;
        for (int i = 0; i < count; ++i)
        {
            const int index = (frontIndex + i) % capacity;
            if (GetQuadrant(buffer[index]) == quadrant)
            {
                matchOffset = i;
                break;
            }
        }

        const int matchIndex = (frontIndex + matchOffset) % capacity;
        point = buffer[matchIndex];

        for (int i = matchOffset; i < count - 1; ++i)
        {
            const int from = (frontIndex + i + 1) % capacity;
            const int to = (frontIndex + i) % capacity;
            buffer[to] = buffer[from];
        }

        rearIndex = (rearIndex - 1 + capacity) % capacity;
        --count;

        lk.unlock();
        notFull.notify_one();
        return true;
    }

private:
    bool HasPointForQuadrant(int quadrant) const
    {
        for (int i = 0; i < count; ++i)
        {
            const int index = (frontIndex + i) % capacity;
            if (GetQuadrant(buffer[index]) == quadrant)
            {
                return true;
            }
        }
        return false;
    }
};

struct QuarterResult
{
    int quarter = 0;
    Point2D pointA;
    Point2D pointB;
    double closestDistance = std::numeric_limits<double>::max();
    int totalPoints = 0;
    bool hasPair = false;
};

QuarterResult ConsumerFunctionAsync(CircularBuffer& buffer, int quarter)
{
    std::vector<Point2D> localPoints;
    Point2D point;

    while (buffer.PopForQuadrant(quarter, point))
    {
        localPoints.push_back(point);
    }

    QuarterResult result;
    result.quarter = quarter;
    result.totalPoints = static_cast<int>(localPoints.size());

    if (localPoints.size() < 2)
    {
        return result;
    }

    for (size_t i = 0; i < localPoints.size() - 1; ++i)
    {
        for (size_t j = i + 1; j < localPoints.size(); ++j)
        {
            const double dist = Distance(localPoints[i], localPoints[j]);
            if (dist < result.closestDistance)
            {
                result.closestDistance = dist;
                result.pointA = localPoints[i];
                result.pointB = localPoints[j];
                result.hasPair = true;
            }
        }
    }

    return result;
}

void ProducerFunction(CircularBuffer& buffer)
{
    std::mt19937 randomGenerator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> pointDistribution(-1000.0f, 1000.0f);
    const auto sleepTime = std::chrono::milliseconds(30);

    for (int i = 0; i < 10000; ++i)
    {
        Point2D point;
        point.x = pointDistribution(randomGenerator);
        point.y = pointDistribution(randomGenerator);
        buffer.Push(point);
        std::this_thread::sleep_for(sleepTime);
    }

    done = true;
    buffer.notEmpty.notify_all();
}

void PrintQuarterResult(const QuarterResult& result)
{
    std::cout << std::fixed << std::setprecision(2);

    if (!result.hasPair)
    {
        std::cout << "quarter " << result.quarter
            << ": not enough points to compute closest pair. Total number of point in this quarter is "
            << result.totalPoints << ".\n";
        return;
    }

    std::cout << "quarter " << result.quarter
        << ": closest points are (" << result.pointA.x << ", " << result.pointA.y
        << ") and (" << result.pointB.x << ", " << result.pointB.y
        << ") and their distance is " << result.closestDistance
        << ". Total number of point in this quarter is " << result.totalPoints << ".\n";
}

int main()
{
    std::cout << "Running " << std::thread::hardware_concurrency() << " threads \n";
    std::cout << "I promise it's working! Waiting...\n";
    std::cout << "Approximate time: 5 minute\n";

    CircularBuffer dataBuffer(400);

    std::future<QuarterResult> quarter1 = std::async(std::launch::async, ConsumerFunctionAsync, std::ref(dataBuffer), 1);
    std::future<QuarterResult> quarter2 = std::async(std::launch::async, ConsumerFunctionAsync, std::ref(dataBuffer), 2);
    std::future<QuarterResult> quarter3 = std::async(std::launch::async, ConsumerFunctionAsync, std::ref(dataBuffer), 3);
    std::future<QuarterResult> quarter4 = std::async(std::launch::async, ConsumerFunctionAsync, std::ref(dataBuffer), 4);

    std::thread producer(ProducerFunction, std::ref(dataBuffer));

    producer.join();

    QuarterResult result1 = quarter1.get();
    QuarterResult result2 = quarter2.get();
    QuarterResult result3 = quarter3.get();
    QuarterResult result4 = quarter4.get();

    PrintQuarterResult(result1);
    PrintQuarterResult(result2);
    PrintQuarterResult(result3);
    PrintQuarterResult(result4);

    return 0;
}
