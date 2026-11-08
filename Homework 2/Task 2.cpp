#include <algorithm>
#include <cstddef>
#include <functional>
#include <future>
#include <iterator>
#include <thread>
#include <utility>
#include <atomic>
#include <iostream>
#include <vector>

namespace parallel 
{

    namespace detail 
    {

        template <typename Iterator, typename Func>
        void for_each_impl(Iterator first, Iterator last, Func& f,
            int depth, std::size_t min_block)
        {
            const auto n = std::distance(first, last);
            if (n <= 0) return;

            if (depth <= 0 || static_cast<std::size_t>(n) < min_block) 
            {
                std::for_each(first, last, std::ref(f));
                return;
            }
            // Если объём задачи меньше установленного минимума, вместо разбивки на два потока мы сразу будем делать её в один, ибо КПД важнее стиля
            Iterator mid = first;
            std::advance(mid, n / 2);

            auto future = std::async(std::launch::async,
                [first, mid, &f, depth, min_block]() 
                {
                    for_each_impl(first, mid, f, depth - 1, min_block);
                });

            for_each_impl(mid, last, f, depth - 1, min_block);
            future.get();
        }

    }
    
    template <typename Iterator, typename Func>
    void for_each(Iterator first, Iterator last, Func& f)
    {
        const auto n = std::distance(first, last);
        if (n <= 0) return;

        unsigned hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 2;
        int depth = 0;
        while ((1u << depth) < hw) ++depth;

        // Та же самая песня: если задача не слишком объёмная, её выгоднее решать в один поток, нежели тратить ресурсы на разбивку.
        constexpr std::size_t kMinBlock = 32;
        const std::size_t target = static_cast<std::size_t>(n) / (std::size_t(1) << depth);
        const std::size_t min_block = std::max(kMinBlock, target);

        detail::for_each_impl(first, last, f, depth, min_block);
    }

} 

// Вообще, по заданию надо написать чисто функцию. Её проверка не заявлена и её, по идее, можно было вообще не писать или вовсе вывести в одельный файл, 
// Но я решил её по итогу добавить, чтобы было наглядно видно, что всё работает. Конечно, можно было просто юнит-тесты вписать, но юзер-инпут кмк время экономит
// Типа, если тебе нужна просто работоспособность - запустил среднюю по палате ситуацию и проверил

int main()
{
    int a = 0, b = 0;
    std::cout << "Введите два числа: ";
    if (!(std::cin >> a >> b)) 
    {
        std::cerr << "Некорректный ввод\n";
        return 1;
    }

    std::vector<int> v{ a, b };

    std::atomic<long long> sum{ 0 };

    auto add = [&sum](int x) 
        {
            sum.fetch_add(x, std::memory_order_relaxed);
        };

    parallel::for_each(v.begin(), v.end(), add);

    std::cout << "a = " << a << '\n'
        << "b = " << b << '\n'
        << "a + b = " << sum.load() << '\n'
        << "проверка: " << a << " + " << b << " = " << (a + b) << '\n';

    return (sum.load() == static_cast<long long>(a) + b) ? 0 : 2;
}