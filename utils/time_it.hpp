#pragma once

#include "chrono"
#include <cstdint>

namespace utils {

    /**
     * @brief pomiar czasu w dowolnej podanej jednostce timeunit
     * 
     * @tparam time_unit std::chrono::... milliseconds microseconds days...
     */
    template <typename time_unit>
    struct time_it {
        using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

        time_point _start {};

        auto set()
        {
            _start = timestamp();
        }

        auto measure() const
            -> typename time_unit::rep
        {
            auto duration = timestamp() - _start;
            auto&& casted = std::chrono::duration_cast<time_unit>(duration);
            return casted.count();
        }

        auto timestamp() const
        {
            return std::chrono::high_resolution_clock::now();
        }
    };
}