// A2_AdvancedOpenMP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

Matrix CreateMatrix(int rows, int cols)
{
    Matrix matrix(rows, std::vector<double>(cols, 0.0));

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            matrix[i][j] = static_cast<double>((i + j) % 100) / 10.0;
        }
    }

    return matrix;
}

// Measure serial runtime
void MultiplySerial(const Matrix& a, const Matrix& b, Matrix& c)
{
    const int m = static_cast<int>(a.size());
    const int n = static_cast<int>(a[0].size());
    const int p = static_cast<int>(b[0].size());

    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < p; ++j)
        {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
            {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}

// Measure parallel runtime
void MultiplyParallelManualRows(const Matrix& a, const Matrix& b, Matrix& c)
{
    const int m = static_cast<int>(a.size());
    const int n = static_cast<int>(a[0].size());
    const int p = static_cast<int>(b[0].size());

#pragma omp parallel
    {
        const int id = omp_get_thread_num();
        const int threadCount = omp_get_num_threads();

        const int rowsPerThread = m / threadCount;
        const int rowStart = id * rowsPerThread;

        const int rowEnd = (id == threadCount - 1) ? m : (rowStart + rowsPerThread);

        for (int i = rowStart; i < rowEnd; ++i)
        {
            for (int j = 0; j < p; ++j)
            {
                double sum = 0.0;
                for (int k = 0; k < n; ++k)
                {
                    sum += a[i][k] * b[k][j];
                }
                c[i][j] = sum;
            }
        }
    }
}

void PrintOutput(const Matrix& matrix, const std::string& title)
{
    std::cout << title << " (first 5x5):\n";
    const int rowsToPrint = std::min(5, static_cast<int>(matrix.size()));
    const int colsToPrint = std::min(5, static_cast<int>(matrix[0].size()));

    for (int i = 0; i < rowsToPrint; ++i)
    {
        for (int j = 0; j < colsToPrint; ++j)
        {
            std::cout << std::setw(10) << std::fixed << std::setprecision(2) << matrix[i][j] << " ";
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

// Compare efficency of serial and parallel versions
double MaxDifference(const Matrix& a, const Matrix& b)
{
    double maxDiff = 0.0;

    for (size_t i = 0; i < a.size(); ++i)
    {
        for (size_t j = 0; j < a[0].size(); ++j)
        {
            maxDiff = std::max(maxDiff, std::abs(a[i][j] - b[i][j]));
        }
    }

    return maxDiff;
}

int main()
{
    const int m = 600;
    const int n = 600;
    const int p = 600;

    const Matrix a = CreateMatrix(m, n);
    const Matrix b = CreateMatrix(n, p);

    Matrix cSerial(m, std::vector<double>(p, 0.0));
    Matrix cParallel(m, std::vector<double>(p, 0.0));

    const int threadsToUse = omp_get_max_threads();
    omp_set_num_threads(threadsToUse);

    std::cout << "Matrix size: " << m << " x " << n << " multiplied by " << n << " x " << p << '\n';
    std::cout << "OpenMP threads requested: " << threadsToUse << "\n\n";

    // measure serial runtime
    const auto serialStart = std::chrono::steady_clock::now();
    MultiplySerial(a, b, cSerial);
    const auto serialEnd = std::chrono::steady_clock::now();

    // measure parallel runtime
    const auto parallelStart = std::chrono::steady_clock::now();
    MultiplyParallelManualRows(a, b, cParallel);
    const auto parallelEnd = std::chrono::steady_clock::now();

    const auto serialUs = std::chrono::duration_cast<std::chrono::microseconds>(serialEnd - serialStart).count();
    const auto parallelUs = std::chrono::duration_cast<std::chrono::microseconds>(parallelEnd - parallelStart).count();

    // Print 5x5 Output for both serial and Parallel multi methods
    PrintOutput(cSerial, "Serial output matrix");
    PrintOutput(cParallel, "Parallel output matrix");

    std::cout << "Serial time:   " << serialUs << " microseconds\n";
    std::cout << "Parallel time: " << parallelUs << " microseconds\n";

    // Compare efficiency
    if (parallelUs > 0)
    {
        std::cout << "Speedup (serial/parallel): " << static_cast<double>(serialUs) / static_cast<double>(parallelUs) << "x\n";
    }

    std::cout << "Max difference between serial and parallel results: " << MaxDifference(cSerial, cParallel) << '\n';

    return 0;
}
