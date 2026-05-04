

#include <iostream>
#include <omp.h>


int main()
{
	omp_set_num_threads(10);
#pragma omp parallel 
	{	
		int ID{ omp_get_thread_num() };
		std::cout << "Hello world from thread with id=" << ID << std::endl;
	}
	std::cout << "back into single thread state\n";
	return  0;

}