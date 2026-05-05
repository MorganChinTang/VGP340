
#include <mutex>
#include <iostream>
#include <random>
#include <chrono>

std::atomic_bool done{ false }; // thread safe flag to indicate data input is finished.

struct CircularBuffer {
	int* buf; 
	int capacity;

	int frontIdx, rearIdx, count;
	std::mutex m;
	std::condition_variable notEmpty, notFull;
	CircularBuffer(int cap) : capacity(cap), frontIdx(0), rearIdx(0), count(0), buf(new int[cap]) {}

	~CircularBuffer() { delete[] buf; }

	void push(int num)
	{
		std::unique_lock<std::mutex> lk(m);
		notFull.wait(lk, [this]() {return count != capacity; });
		buf[rearIdx] = num;
		rearIdx = (rearIdx + 1) % capacity;
		++count;
		
		lk.unlock();
		notEmpty.notify_one();
	}

	int pop()
	{
		std::unique_lock<std::mutex> lk(m);
		notEmpty.wait(lk, [this]() {return count > 0; });
		int data{ buf[frontIdx] };
		frontIdx = (frontIdx + 1) % capacity;
		--count;

		lk.unlock();
		notFull.notify_one();
		return data;
	}

};

std::mutex mMmutex;
int totalMin{ INT_MAX }, totalMax{ INT_MIN };

void consumer(CircularBuffer& buf, int id)
{
	int local_min{ INT_MAX };
	int local_max{ INT_MIN };
	for (int i = 0; i < 950; ++i) {
		int val{ buf.pop() };
		local_min = std::min(local_min, val);
		local_max = std::max(local_max, val);
	}
	{
		std::lock_guard<std::mutex> Mmlk(mMmutex);
		std::cout << "Consumer " << id << ": local_Max value is " << local_max << ", and local_min value is " << local_min << std::endl;
		totalMin = std::min(totalMin, local_min);
		totalMax = std::max(totalMax, local_max);
	}	
}

void producer(CircularBuffer& buf, int id)
{
	// initialize a random generator
	std::mt19937 generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());

	for (int i = 0; i < 1000; ++i)
	{
		buf.push(generator()%1000);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));  // just mimicking some work to be done.
	}
}

int main()
{
	CircularBuffer dataBuf(400);
	std::thread consumer1(consumer, std::ref(dataBuf), 1);
	std::thread consumer2(consumer, std::ref(dataBuf), 2);
	std::thread producer1(producer, std::ref(dataBuf), 1);
	std::thread producer2(producer, std::ref(dataBuf), 2);

	consumer1.join();
	consumer2.join();
	producer1.join();
	producer2.join();

	// print the final min and max of the data in the buf,
	std::cout << "Total min = " << totalMin << ", totalMax = " << totalMax << std::endl;
	return 0;

}