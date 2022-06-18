#pragma once

// thank you riptutorial.com <3

#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <vector>

namespace utils {

struct thread_pool {
    std::mutex m;
    std::condition_variable v;

    std::deque<std::packaged_task<void()>> tasks;
    std::vector<std::future<void>> thread_futures;

    thread_pool(std::size_t threads_number = 1)
    {
        for (std::size_t thread = 0; thread < threads_number; ++thread) {
            thread_futures.push_back(std::async(std::launch::async, [this] {
                thread_task();
            }));
        }
    }

    ~thread_pool()
    {
        stop();
    }

    template <class Task, class ReturnType = std::invoke_result_t<Task&>>
    auto queue(Task&& f) -> std::future<ReturnType>
    {
        std::packaged_task<ReturnType()> p(std::forward<Task>(f));
        auto r = p.get_future();

        {
            std::unique_lock<std::mutex> l(m);

            tasks.emplace_back(std::move(p));
        }

        v.notify_one();

        return r;
    }

    void abort()
    {
        cancel_pending();
        stop();
    }

    void cancel_pending()
    {
        std::unique_lock<std::mutex> l(m);
        tasks.clear();
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> l(m);

            for (auto&& _ [[maybe_unused]] : thread_futures) {
                tasks.push_back({});
            }
        }

        v.notify_all();
        thread_futures.clear();
    }

private:
    void thread_task()
    {
        while (true) {
            std::packaged_task<void()> f;
            {
                std::unique_lock<std::mutex> l(m);

                if (tasks.empty()) {
                    v.wait(l, [&] { return !tasks.empty(); });
                }

                f = std::move(tasks.front());
                tasks.pop_front();
            }

            if (!f.valid()) {
                return;
            }

            f();
        }
    }
};

}
