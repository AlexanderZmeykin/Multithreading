#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <string>
#include <limits>

static std::once_flag g_hardwareFlag;

// Функция вычислений на участке
static void workerAdd(const std::vector<double>& a,
    const std::vector<double>& b,
    std::vector<double>& c,
    std::size_t start,
    std::size_t end)
{
    std::call_once(g_hardwareFlag, []() 
        {
        unsigned hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 1;
        std::cout << "Доступное количество аппаратных ядер: " << hw << "\n";
        std::cout.flush();
        });

    for (std::size_t i = start; i < end; ++i)
        c[i] = a[i] + b[i];
}

// Функция сложения векторов
static void parallelAdd(const std::vector<double>& a,
    const std::vector<double>& b,
    std::vector<double>& c,
    int numThreads)
{
    const std::size_t n = a.size();
    const std::size_t chunk = (n + static_cast<std::size_t>(numThreads) - 1) / static_cast<std::size_t>(numThreads);

    std::vector<std::thread> threads;
    threads.reserve(static_cast<std::size_t>(numThreads));

    for (int t = 0; t < numThreads; ++t) 
    {
        std::size_t start = static_cast<std::size_t>(t) * chunk;
        if (start >= n) break;
        std::size_t end = std::min(start + chunk, n);

        threads.emplace_back([&a, &b, &c, start, end]() 
            { 
                workerAdd(a, b, c, start, end);
            });
    }
    for (auto& th : threads) th.join();
}

int main()
{
    // Именно этот кролик из ШоШа пропечатает нам число ядер.
    {
        std::vector<double> a(8, 1.0), b(8, 2.0), c(8, 0.0);
        parallelAdd(a, b, c, 2);
    }
    std::cout << "\n";

    const std::vector<std::size_t> sizes{ 1000, 10000, 100000, 1000000 };
    const std::vector<int> threadNums{ 1, 2, 4, 8, 16 };
    const int warmupIters = 3;
    const int measureIters = 20;

    // Заголовок таблицы
    std::cout << std::left
        << std::setw(12) << "Размер"
        << std::setw(10) << "Потоки"
        << std::setw(18) << "Ср. время (мс)"
        << std::setw(12) << "Ускорение"
        << "\n";
    std::cout << std::string(58, '-') << "\n";

    for (std::size_t n : sizes) 
    {
        // Инициализация входных векторов
        std::vector<double> a(n), b(n), c(n);
        for (std::size_t i = 0; i < n; ++i) 
        {
            a[i] = static_cast<double>(i % 1000);
            b[i] = static_cast<double>(n - i) * 0.5;
        }

        double baseline = 0.0;
        double bestTime = std::numeric_limits<double>::max();
        int bestThreads = 0;

        for (int tc : threadNums) 
        {
            for (int k = 0; k < warmupIters; ++k) // Т.н. "прогрев"
                parallelAdd(a, b, c, tc);

            double total = 0.0;
            for (int k = 0; k < measureIters; ++k) 
            {
                auto t0 = std::chrono::high_resolution_clock::now();
                parallelAdd(a, b, c, tc);
                auto t1 = std::chrono::high_resolution_clock::now();
                total += std::chrono::duration<double, std::milli>(t1 - t0).count();
            }
            double avg = total / static_cast<double>(measureIters);

            if (tc == 1) baseline = avg;
            double speedup = (baseline > 0.0) ? baseline / avg : 1.0;

            if (avg < bestTime) {bestTime = avg; bestThreads = tc;}

            std::cout << std::left
                << std::setw(12) << n
                << std::setw(10) << tc
                << std::setw(18) << std::fixed << std::setprecision(4) << avg
                << std::setw(12) << std::setprecision(2) << speedup
                << "\n";
        }

        std::cout << "  -> Лучший вариант для " << n << " элементов: "
            << bestThreads << " поток(ов), "
            << std::fixed << std::setprecision(4) << bestTime << " мс\n\n";
    }

    return 0;
}