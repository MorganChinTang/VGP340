// VGP340_1.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <omp.h>
#include <iostream>
#include <chrono>
#include <algorithm>

#define NUM_THREADS 4
static long numSteps = 1000000;
double step = 0.0;


void Exercise1()
{
    int testValue = 0;
    //omp_set_num_threads(4);
#pragma omp parallel num_threads(4)
    {
        int id = omp_get_thread_num();
        ++testValue;
        printf(" Hello(%d, %d)", id, testValue);
        ++testValue;
        printf(" World(%d, %d)\n", id, testValue);
    }
}

void Exercise2()
{
    int numThreads = 0;
    double x = 0.0;
    double pi = 0.0;
    double sum[NUM_THREADS] = { 0.0 };  // this causes "false sharing"
    step = 1.0 / (double)numSteps;
    omp_set_num_threads(NUM_THREADS); // sets the number of parallel threads

#pragma omp parallel // copies the code for each thread
    {
        int i = 0;
        int id = omp_get_thread_num();
        int nThreads = omp_get_num_threads();

        if (id == 0)
        {
            numThreads = nThreads;
        }
        for (i = id; i < numSteps; i += nThreads)
        {
            x = (i + 0.5) * step;
            sum[id] += (4.0 / (1.0 + x * x));
        }
    }

    for (int i = 0; i < numThreads; ++i)
    {
        std::cout << "ID: " << i << " Sum: " << sum[i] << "\n";
        pi += step * sum[i];
    }
    std::cout << "PI: " << pi << "\n";
}
void Exercise3()
{
    double pi = 0.0;
    step = 1.0 / (double)numSteps;
    omp_set_num_threads(NUM_THREADS); // sets the number of parallel threads

#pragma omp parallel // copies the code for each thread
    {
        double x = 0.0;
        double sum = 0.0;
        int i = 0;
        int id = omp_get_thread_num();
        int nThreads = omp_get_num_threads();

        for (i = id; i < numSteps; i += nThreads)
        {
            x = (i + 0.5) * step;
            sum += (4.0 / (1.0 + x * x));
        }

#pragma omp critical // letting thread know a shared value is being updated
        {
            std::cout << "ID: " << id << " Sum: " << sum << "\n";
            pi += sum * step;
            std::cout << "ID: " << id << " Current PI: " << pi << "\n";
        }
    }
    std::cout << "PI: " << pi << "\n";
}
void Exercise4()
{
    double x = 0.0;
    double pi = 0.0;
    double sum = 0.0;

    step = 1.0 / (double)numSteps;
    omp_set_num_threads(NUM_THREADS); // sets the number of parallel threads

#pragma omp parallel for private(x) reduction(+:sum) // copies the code for each thread, copies the x, combines the sum at the end

    for (int i = 0; i < numSteps; ++i)
    {
        x = (i + 0.5) * step;
        sum += (4.0 / (1.0 + x * x));
    }

    pi = sum * step;
    std::cout << "PI: " << pi << "\n";
}

int main()
{
    std::cout << "Enter Num Steps: ";
    std::cin >> numSteps;

    double x = 0.0;
    double pi = 0.0;
    double sum = 0.0;
    step = 1.0 / (double)numSteps;
    // the current cpu time in utc
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < numSteps; ++i)
    {
        x = (i + 0.5) * step;
        sum += (4.0 / (1.0 + x * x));
    }

    pi = sum * step;
    std::cout << "PI: " << pi << "\n";
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
    int duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "Duration: " << duration << "\n";

    return 0;
}
