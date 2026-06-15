// A5_MonteCarloPi.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <limits>
#include <omp.h>
#include <chrono>

// SERIAL PI

/**
* f(x) = 4.0/(1 + x^2)
* numerical integration of f(x) over range of 0 to 1 which should be equal to PI
**/
long num_steps{ 10000000 };
const double M_PI{ 3.14159265358979323846 };
int main()
{
    auto startTime{ std::chrono::steady_clock::now() };
    const double step{ 1.0 / (double)num_steps };
    double sum{ 0.0 };
    const int num_threads{ omp_get_max_threads() };
    //double* sums{ new double[2*num_threads] };
    int num_runs{ 10 };
    for (int k{ 0 }; k < num_runs; ++k) {  // repeating the computation num_runs times
#pragma omp parallel
        {
            int id{ omp_get_thread_num() };
            int iStart{ (id * num_steps) / num_threads };
            int iEnd{ ((id + 1) * num_steps) / num_threads };
            //std::cout << "iStart=" << iStart << ", iEnd=" << iEnd << std::endl;
            if (id == num_threads - 1)
                iEnd = num_steps;
            double localsum{ 0.0 };
            //sums[id] = 0.0;
            for (int i{ iStart }; i < iEnd; ++i)
            {
                double x = (i + 0.5) * step;
                localsum += 4.0 / (1.0 + x * x);
            }
#pragma omp critical
            sum += localsum;
        }
    }
    //	for (int i = 0; i < num_threads; ++i)  // removing false sharing by replace sums by localsum
    //		sum += sums[i];

    double pi{ sum * step };
    pi = pi / double(num_runs);
    auto endTime{ std::chrono::steady_clock::now() };
    std::cout.precision(15);
    std::cout << "PI for num_steps = " << num_steps << "num_threads=" << num_threads << " is " << pi << " vs exact PI=" << M_PI << std::endl;
    std::cout << "Elapsed time in milliseconds: " << std::chrono::duration_cast<std::chrono::milliseconds>
        (endTime - startTime).count() / num_runs << std::endl;
}


//OMP PI


/**
* f(x) = 4.0/(1 + x^2)
* numerical integration of f(x) over range of 0 to 1 which should be equal to PI
**/
long num_steps{ 10000000 };
const double M_PI{ 3.14159265358979323846 };
int main()
{
    auto startTime{ std::chrono::steady_clock::now() };
    double step{ 1.0 / (double)num_steps };
    double sum{ 0.0 };
    int num_threads{ omp_get_max_threads() };
    int num_runs{ 10 };
    for (int k{ 0 }; k < num_runs; ++k) {  // repeating the computation num_runs times
#pragma omp parallel for reduction(+: sum)		
        for (int i{ 0 }; i < num_steps; ++i)
        {
            double x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
    }

    double pi{ sum * step };

    auto endTime{ std::chrono::steady_clock::now() };
    std::cout.precision(15);
    std::cout << "PI for num_steps = " << num_steps << "num_threads=" << num_threads << " is " << pi << " vs exact PI=" << M_PI << std::endl;
    std::cout << "Elapsed time in milliseconds: " << std::chrono::duration_cast<std::chrono::milliseconds>
        (endTime - startTime).count() / num_runs << std::endl;
}