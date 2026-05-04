// Week2.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <omp.h>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <iomanip> // for std::setprecision

#define NUM_THREADS 4
static long numSteps = 1000000;
double step = 0.0;
double RandRange(double min, double max)
{
    double r = static_cast<double>(rand())/RAND_MAX;
    return min + (max - min) * r;
}


int monteCarlo()
{
    // monte carlo pi calculation
    long numTrials = 10000;
    long i = 0;
    long nCirc = 0;
    double pi = 0.0;
    double x = 0.0;
    double y = 0.0;
    double r = 1.0; // radius of circle. Side of square is 2*r
    std::srand(time(0));

#pragma omp parallel for private(x,y) reduction(+:nCirc)
    for (i = 0; i < numTrials; ++i)
    {
        x = RandRange(-r, r);
        y = RandRange(-r, r);
        // distSquare for 2D point
        if (x * x + y * y <= r * r)
        {
            ++nCirc;
        }
    }
    pi = 4.0 * (static_cast<double>(nCirc) 
        / static_cast<double>(numTrials));
    std::cout << std::setprecision(20) << "PI: " << pi << "\n";
    std::cout << "NumHits: " << nCirc << "\n";

    return 0;
}

int Excercise2()
{
    omp_set_num_threads(4);

#pragma omp parallel sectionsS
    {

#pragma omp section
            {
                int id = omp_get_thread_num();
                for(int i = 0; i < 10; i++)
                {
                    std::cout << "Section1 for ID:" << id << " Index:" << i << "\n";
                }
            }

#pragma omp section
            {
                int id = omp_get_thread_num();
                for(int i = 0; i < 10; i++)
                {
                    std::cout << "Section2 for ID:" << id << " Index:" << i << "\n";
                }
            }
    }

    return 0;
}

int main()
{
    Excercise2();
    //monteCarlo();

}