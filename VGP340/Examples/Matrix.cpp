#include <memory>
#include <random>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <thread>
#include <assert.h>

struct Matrix
{
	int nRows, nCols;
	long long **data;

	Matrix() = default;
	Matrix(int r, int c): nRows{r}, nCols{c}
	{
		data = new long long *[nRows];
		for (int i = 0; i < nRows; ++i)
		{
			data[i] = new long long[nCols];
		}
	}
	~Matrix()
	{
		if (nRows == 0)
			return;
		for (int i = 0; i < nRows; ++i)
			delete[] data[i];
		delete[] data;
	}
	// initialize the matrix by random numbers
	void init()
	{
		std::srand(std::time(0));
		for (int i = 0; i < nRows; ++i)
			for (int j = 0; j < nCols; ++j)
				data[i][j] = std::rand() % 20 - 10;
	}

	void print()
	{
		std::cout << "Matrix:\n";
		for (int i = 0; i < nRows; ++i)
		{
			for (int j = 0; j < nCols; ++j)
				std::cout << data[i][j] << ", ";
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	static void Mult(Matrix const& A, Matrix const& B, Matrix* result)
	{
		//checking if A's num of Col is equal to B's num of Row
		assert(A.nCols == B.nRows && "Matrices num of Cols and Rows do not match!");
		if (A.nCols != B.nRows)
		{
			std::cout << "A and B dimensions do not match!" << 
				" A.nCols=" << A.nCols << ", B.nRows=" << B.nRows << std::endl;
			return;
		}
		
		// do the multiplication
		for (int i = 0; i <= A.nRows; ++i)
		{
			for (int j = 0; j < B.nCols; ++j)
			{
				long long res = 0;
				for (int k = 0; k < A.nCols; ++k)
					res += A.data[i][k] * B.data[k][j];
				result->data[i][j] = res;
			}
		}
	}
};

