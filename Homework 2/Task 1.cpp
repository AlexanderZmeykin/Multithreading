#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <utility>

void findMinTask(std::promise<size_t> promise,
    const std::vector<int>& arr,
    size_t begin, size_t end) 
{
    try 
    {
        size_t minIdx = begin;
        for (size_t i = begin + 1; i < end; ++i) 
        {
            if (arr[i] < arr[minIdx]) 
            {
                minIdx = i;
            }
        }
        promise.set_value(minIdx);     
    }
    catch (...) 
    {
        promise.set_exception(std::current_exception());
    }
}

void selectionSort(std::vector<int>& arr) 
{
    const size_t n = arr.size();
    if (n < 2) return;

    for (size_t i = 0; i + 1 < n; ++i) 
    {
        std::promise<size_t> promise;
        std::future<size_t>  future = promise.get_future();
        std::thread worker(findMinTask,std::move(promise),std::cref(arr),i,n);
        const size_t minIdx = future.get();
        worker.join();
        if (minIdx != i) 
        {
            std::swap(arr[i], arr[minIdx]);
        }
    }
}

int main() 
{
    std::vector<int> data = { 5, 3, 8, 1, 9, 2, 7, 4, 6, 0 };

    std::cout << "До сортировки:    ";
    for (int x : data) std::cout << x << ' ';
    std::cout << '\n';

    selectionSort(data);

    std::cout << "После сортировки: ";
    for (int x : data) std::cout << x << ' ';
    std::cout << '\n';

    return 0;
}