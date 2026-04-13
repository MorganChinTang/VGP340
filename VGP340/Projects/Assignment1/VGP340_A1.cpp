// VGP340_A1.cpp : OpenMP PI Computing Assignment

#include <omp.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

using Float = double;

Float SerialPI_Integration(int n)
{
    const Float step = 1.0 / static_cast<Float>(n);
    Float sum = 0.0;

    for (int i = 0; i < n; ++i)
    {
        const Float x = (static_cast<Float>(i) + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    return sum * step;
}

Float ParallelPI_Integration(int n)
{
    const Float step = 1.0 / static_cast<Float>(n);
    std::vector<Float> terms(n, 0.0);

#pragma omp parallel for
    for (int i = 0; i < n; ++i)
    {
        const Float x = (static_cast<Float>(i) + 0.5) * step;
        terms[i] = 4.0 / (1.0 + x * x);
    }

    Float sum = 0.0;
    for (int i = 0; i < n; ++i)
    {
        sum += terms[i];
    }

    return sum * step;
}

int main()
{
    const int testNs[] = { 10000, 100000, 1000000 };

    std::cout << std::fixed << std::setprecision(15);

    for (int n : testNs)
    {
        const auto serialStart = std::chrono::steady_clock::now();
        const Float piSerial = SerialPI_Integration(n);
        const auto serialEnd = std::chrono::steady_clock::now();
        const auto serialMs = std::chrono::duration_cast<std::chrono::milliseconds>(serialEnd - serialStart).count();

        const auto parallelStart = std::chrono::steady_clock::now();
        const Float piParallel = ParallelPI_Integration(n);
        const auto parallelEnd = std::chrono::steady_clock::now();
        const auto parallelMs = std::chrono::duration_cast<std::chrono::milliseconds>(parallelEnd - parallelStart).count();

        std::cout << "n = " << n << "\n";
        std::cout << "PI_serial_" << n << " = " << piSerial << "\n";
        std::cout << "Serial elapsed time: " << serialMs << " ms\n";
        std::cout << "PI_parallel_" << n << " = " << piParallel << "\n";
        std::cout << "Parallel elapsed time: " << parallelMs << " ms\n";
        std::cout << "PI values are exactly the same: " << ((piSerial == piParallel) ? "Yes" : "No") << "\n";
        std::cout << "\n\n";
    }

    return 0;


}
