#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>

int  g_max_clients = 5;     // Параметр: сколько всего клиентов придёт
int  g_queue = 0;     // Счётчик клиентов в очереди
bool g_client_done = false; // Свитч на приход клиента. Когда станет true, тогда настанет конец очереди

std::mutex g_mtx;
std::condition_variable g_cv;

void client_thread()
{
    for (int i = 0; i < g_max_clients; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            ++g_queue;
            std::cout << "[Клиент] пришёл. В очереди: " << g_queue << std::endl;
        }
        g_cv.notify_all();
    }

    {
        std::lock_guard<std::mutex> lock(g_mtx);
        g_client_done = true;
        std::cout << "[Клиент] больше клиентов не будет." << std::endl;
    }
    g_cv.notify_all();
}

void operator_thread()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(g_mtx);

        g_cv.wait(lock, [] { return g_queue > 0 || g_client_done; });

        // Данный if будет брейкать команду только при условии того, что в очереди людей не осталось и работа с последним завершена
        if (g_queue == 0 && g_client_done)
        {
            std::cout << "[Операционист] очередь пуста, клиентов больше нет." << std::endl;
            break;
        }

        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        lock.lock();

        if (g_queue > 0)
        {
            --g_queue;
            std::cout << "[Операционист] обслужил клиента. В очереди: " << g_queue << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        int v = std::atoi(argv[1]);
        if (v > 0) g_max_clients = v;
    }

    std::cout << "Максимальное количество клиентов: " << g_max_clients << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    std::thread t1(client_thread);
    std::thread t2(operator_thread);

    t1.join();
    t2.join();

    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Работа завершена. В очереди осталось: " << g_queue << std::endl;
    return 0;
}