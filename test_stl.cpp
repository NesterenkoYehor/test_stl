#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

template <class Container>
void fill1(Container &container, int size)
{
    for (int i = 0; i < size; i++)
        container.push_back(rand());
}

template <class Container>
void fill2(Container &container, int size)
{
    for (int i = 0; i < size; i++)
        container.insert(rand());
}

template <class Container>
void find1(Container &container, int element)
{
    for (auto it : container)
        if (it == element)
            return;
}

template <class Container>
void find2(Container &container, int element)
{
    container.find(element);
}

template <class Container>
void erase(Container &container)
{
    container.clear();
}

std::chrono::duration<int, std::nano> timer(std::function<void()> func)
{
    auto start = std::chrono::steady_clock::now();

    func();

    return std::chrono::steady_clock::now() - start;
}

bool could_insert = true;
std::mutex insert_in_stream1;
std::mutex insert_in_stream2;
std::condition_variable stream_check;

template<class Container>
void work1(Container &container, std::string name, std::stringstream &stream, int min, int max)
{
    std::vector<std::chrono::duration<int, std::nano>> fill_time;
    std::vector<std::chrono::duration<int, std::nano>> find_time;
    std::vector<std::chrono::duration<int, std::nano>> del_time;

    srand(time(NULL));

    for (int i = 0; i < 1000; i++)
    {
        for (int i = min; i < max; i *= 10)
        {
            fill_time.push_back(timer(std::bind(fill1<Container>, std::ref(container), i)));

            container.clear();
        }

        for (int i = 0; i < 5; i++)
            find_time.push_back(timer(std::bind(find1<Container>, std::ref(container), rand())));

        del_time.push_back(timer(std::bind(erase<Container>, std::ref(container))));
    }

    double avarage_fill_time = 0;
    double avarage_find_time = 0;
    double avarage_del_time = 0;

    for (auto it : fill_time)
        avarage_fill_time += it.count() / (double)fill_time.size();

    for (auto it : find_time)
        avarage_find_time += it.count() / (double)find_time.size();

    for (auto it : del_time)
        avarage_del_time += it.count() / (double)del_time.size();

    std::unique_lock<std::mutex> locker(insert_in_stream1);
    stream_check.wait(locker, [&](){return could_insert;});

    could_insert = false;

    stream << "\n\nContainer " << name << '\n'
           << "Fill time: " << std::setprecision(10) << avarage_fill_time << '\n'
           << "Find time: " << std::setprecision(10) << avarage_find_time << '\n'
           << "Del time: " << std::setprecision(10) << avarage_del_time << '\n';

    could_insert = true;
}

template<class Container>
void work2(Container &container, std::string name, std::stringstream &stream, int min, int max)
{
    std::vector<std::chrono::duration<int, std::nano>> fill_time;
    std::vector<std::chrono::duration<int, std::nano>> find_time;
    std::vector<std::chrono::duration<int, std::nano>> del_time;

    srand(time(NULL));

    for (int i = 0; i < 1000; i++)
    {
        for (int i = min; i < max; i *= 10)
        {
            fill_time.push_back(timer(std::bind(fill2<Container>, std::ref(container), i)));

            container.clear();
        }

        for (int i = 0; i < 5; i++)
            find_time.push_back(timer(std::bind(find2<Container>, std::ref(container), rand())));

        del_time.push_back(timer(std::bind(erase<Container>, std::ref(container))));
    }

    double avarage_fill_time = 0;
    double avarage_find_time = 0;
    double avarage_del_time = 0;

    for (auto it : fill_time)
        avarage_fill_time += it.count() / (double)fill_time.size();

    for (auto it : find_time)
        avarage_find_time += it.count() / (double)find_time.size();

    for (auto it : del_time)
        avarage_del_time += it.count() / (double)del_time.size();

    std::unique_lock<std::mutex> locker(insert_in_stream2);
    stream_check.wait(locker, [&](){return could_insert;});

    could_insert = false;

    stream << "\nContainer " << name << '\n'
           << "Fill time: " << std::setprecision(10) << avarage_fill_time << '\n'
           << "Find time: " << std::setprecision(10) << avarage_find_time << '\n'
           << "Del time: " << std::setprecision(10) << avarage_del_time << '\n';

    could_insert = true;
}

std::mutex output;

void test_work(int min, int max)
{
    std::vector<int> vector;
    std::list<int> list;
    std::set<int> set;
    std::unordered_set<int> uset;
    std::stringstream stream;

    std::thread t1(work1<std::vector<int>>, std::ref(vector), "vector", std::ref(stream), min, max);
    std::thread t2(work1<std::list<int>>, std::ref(list), "list", std::ref(stream), min, max);
    std::thread t3(work2<std::set<int>>, std::ref(set), "set", std::ref(stream), min, max);
    std::thread t4(work2<std::unordered_set<int>>, std::ref(uset), "unordered set", std::ref(stream), min, max);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    output.lock();

    std::cout << "\nMin: " << min << "\tMax: " << max << '\n' << stream.str();

    output.unlock();
}

int main()
{
    std::thread t1(test_work, 10, 1000);
    std::thread t2(test_work, 1000, 100000);

    t1.join();
    t2.join();

    return 0;
}