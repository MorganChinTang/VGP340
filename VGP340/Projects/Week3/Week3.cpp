// Week3.cpp : Dining and Debating Philosophers


#include <iostream>
#include <future>
#include <thread>
#include <sstream>
#include <chrono>
#include <random>
#include <vector>
#include <mutex>
#include "LockFreeStack.h"

void WriteMessage(const std::string& message)
{
    std::cout << message;
}

void Example1()
{
    auto f = std::async(WriteMessage, "Hello World from std::async!\n");
    WriteMessage("Hello World from MAIN\n");
    f.wait();  // must be called or get()
    //f.get();
}

void Example2()
{
    std::thread t(WriteMessage, "Hello World from std::thread\n");
    for (int i = 0; i < 100; ++i)
        WriteMessage("Hello World from MAIN\n");
    t.join();
}

void Example3()
{
    // default is std::launch::async
    // std::launch::async = launch when the async is created
    // std::launch::deferred = launch when the info is requested (f.wait() or f.get());
    auto f = std::async(std::launch::deferred, WriteMessage, "Hello World from std::async\n");
    WriteMessage("Hello World from MAIN\n");
    (void)getchar(); // basically wait for an input before proceeding
    f.wait();
}
int FindTheAnswer()
{
    return 42;
}
void Example4()
{
    auto f = std::async(FindTheAnswer);
    std::cout << "The Meaning Of Life Is: " << f.get() << "\n";
}
std::string CopyString(const std::string& str)
{
    return str;
}
void Example5()
{
    std::string s = "HELLO";
    //auto f = std::async(std::launch::async, CopyString, std::ref(s));
    auto f = std::async(std::launch::deferred, [&s]() { return CopyString(s); });
    s = "GOODBYE";
    std::cout << f.get() << " WORLD!\n";
}

void FindThePromiseAnswer(std::promise<int>* p)
{
    p->set_value(42);
}
void Example6()
{
    std::promise<int> p;
    auto f = p.get_future();
    std::thread t(FindThePromiseAnswer, &p);
    std::cout << "The answer is: " << f.get() << "\n";
    t.join();
}
void Example7()
{
    std::packaged_task<int()> task(FindTheAnswer);
    auto f = task.get_future();
    std::thread t(std::move(task));
    std::cout << "The answer is: " << f.get() << "\n";
    t.join();
}
void WaitForNotify(int id, std::shared_future<int> sf)
{
    std::ostringstream os;
    os << "Thread " << id << " waiting\n";
    std::cout << os.str();
    os.str("");
    os << "Thread " << id << " woken, val=" << sf.get() << "\n";
    std::cout << os.str();
}
void Exercise8()
{
    std::promise<int> p;
    auto sf = p.get_future().share();
    std::thread t1(WaitForNotify, 1, sf);
    std::thread t2(WaitForNotify, 2, sf);
    std::cout << "Waiting\n";
    std::cin.get();
    p.set_value(42);
    t2.join();
    t1.join();
}
std::mutex myMutex;
void MyFunction()
{
    //myMutex.lock();
    //std::cout << "In My Function\n";
    //myMutex.unlock();
    std::lock_guard<std::mutex> myGuard(myMutex);
    std::cout << "In My Function\n";
}
void Example9()
{
    myMutex.lock();
    std::thread t(MyFunction);
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "In Main\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    myMutex.unlock();
    t.join();
}
void Function(int i)
{
    std::unique_lock<std::mutex> guard(myMutex);
    std::cout << "In Function (" << i << ")\n";
    guard.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    guard.lock();
    std::cout << "In Function (" << i << ") AGAIN\n";
}
void Exercise10()
{
    std::unique_lock<std::mutex> guard(myMutex);
    std::thread t1(Function, 1);
    std::thread t2(Function, 2);
    std::cout << "In Main\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    guard.unlock();
    t2.join();
    t1.join();
}
class Account
{
public:
    Account(int balance) : balance(balance) {}
    friend void Transfer(Account& from, Account& to, int amount)
    {
        std::lock_guard<std::mutex> lockFrom(from.m);
        std::lock_guard<std::mutex> lockTo(to.m);
        from.balance -= amount;
        to.balance += amount;

        // using adopt lock, takes current state of lock and applies
        //std::lock(from.m, to.m);
        //std::lock_guard<std::mutex> lockFrom(from.m, std::adopt_lock);
        //std::lock_guard<std::mutex> lockTo(to.m, std::adopt_lock);
        //from.balance -= amount;
        //to.balance += amount;
    }
    int GetBalance() const
    {
        return balance;
    }
private:
    std::mutex m;
    int balance = 0;
};
void Exercise11()
{
    Account accountA(100);
    Account accountB(100);
    Transfer(accountA, accountB, 50);
    std::cout << "Account A has: " << accountA.GetBalance() << "\n";
    std::cout << "Account B has: " << accountB.GetBalance() << "\n";
}


// Lock Free Stack Example
std::mutex gPrintLock;      // used for serializing output stream
bool gDone = false;
LockFreeStack<unsigned> gErrorCodes;    // stack of error codes issued by running threads

// run by logger threads, which pop errors from the stack and process them (print them)
void LoggerFunction()
{
    // print a startup message
    std::unique_lock<std::mutex> lk(gPrintLock);
    std::cout << "[Logger]\t running ....\n";
    lk.unlock();
    // while there are error codes in the stack, process them
    while (!gDone)
    {
        if (!gErrorCodes.Empty())
        {
            auto code = gErrorCodes.Pop();
            if (code != nullptr)
            {
                lk.lock(); // lock to prevent other threads from accessing this
                std::cout << "[Logger]\t processing error " << *code << "\n";
                lk.unlock(); // unlock to make this section available again
            }
        }
    }
}

// run by worker threads, which runs process that may trigger error codes
void WorkerFunction(int id, std::mt19937& randGen) // mt199973 is a random number generator
{
    // print startup message
    std::unique_lock<std::mutex> lk(gPrintLock);
    std::cout << "[Worker " << id << "]\t running ....\n";
    lk.unlock();

    while (!gDone)
    {
        // simulate some work
        std::this_thread::sleep_for(std::chrono::seconds(1 + randGen() % 5));
        // simulate error
        unsigned errorCode = id * 100 + (randGen() % 50); // generate a random "error code"
        gErrorCodes.Push(errorCode);
    }
}

int main()
{
    std::mt19937 randomGenerator((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::vector<std::thread> threads;

    // create all logger functions and add to vector
    for (int i = 0; i < 5; ++i)
    {
        threads.push_back(std::thread(LoggerFunction));
    }
    // create all worker functions and add to vector
    for (int i = 0; i < 5; ++i)
    {
        // add in parameters (id) as well as a reference to the random generator
        //  with the reference, as it is being called, it updates
        //  otherwise each thread will have the exact same random outcomes
        threads.push_back(std::thread(WorkerFunction, i + 1, std::ref(randomGenerator)));
    }

    // simulate running the main thread/application for 30sec
    std::this_thread::sleep_for(std::chrono::seconds(30));
    gDone = true;
    for (auto& t : threads)
    {
        t.join();
    }

    return 0;
}
