/*
* This is serial compuation of PI using inegral of f(x) = 4/(1+x*x)
*/ 
#include <iostream>
#include <iomanip>
#include <chrono>
#include <numeric>
#include <omp.h>

using namespace std;
int NUM{ 10000000 };
constexpr double pi = 3.14159265358979323846;
int main()
{
	auto start{ chrono::steady_clock::now() };
	double step{ 1.0 / (double)NUM };
	double output{};
//#pragma omp parallel
	//{
	//	cout << "Number of threads used: " << omp_get_num_threads() << endl;
		double outputs[16]{};

#pragma omp parallel for reduction(+: output)
		for (int i = 0; i < NUM; ++i)
		{
			int ID{ omp_get_thread_num() };
			double x{ (double(i) + 0.5) * step };
			output += (4.0 / (1 + x * x));
		}
	
	//}
	//for (int i = 0; i < 16; ++i)
	//	output += outputs[i];
	output *= step;

	auto end{ chrono::steady_clock::now() };
	cout << "microseconds:" << chrono::duration_cast<chrono::microseconds>(end - start).count() << endl;
	std::cout <<std::setprecision(20)<< "Actual PI=" << pi;
	std::cout << std::setprecision(20) << ", computed PI = " << output << std::endl;

	return 0;
}