/**
 * create time : 2022-03-08
 * Author : Zheng Shuwen
 * Timestamp hpp
 */
#pragma once

// #include "headers.hpp"
#include "Random.hpp"

namespace gl {
namespace fastsgg {

class Timestamp
{
private:
    int_t min_timestamp;
    int_t max_timestamp;
    Random rand;

public:
    Timestamp () {}

    Timestamp (int_t mit, int_t mat) : min_timestamp(mit), max_timestamp(mat) {}

    int_t genTimestampNum () {
        // TODO
        return 1 + rand.nextInt(1); // 1 or 2
    }

    int_t genTimestamp () {
        int_t bound = max_timestamp - min_timestamp;
        return min_timestamp + rand.nextInt(bound);
    }

    int_t genTimestamp (int_t mit, int_t mat) {
        int_t bound = std::min(max_timestamp, mat) - std::max(min_timestamp, mit);
        return min_timestamp + rand.nextInt(bound);
    }

    int_t genTimestamp (const std::vector<std::vector<int_t>>& window) {
        int_t ans = 0;
        // gen mapped ans
        int_t bound = 0, n = window.size();
        for (auto& win : window)
            bound += win[1] - win[0];
        int_t ans_map = rand.nextInt(bound);
        // gen original ans
        int_t last_sum = 0, sum = window.front()[1] - window.front()[0];
        for (int_t i = 1; i < n; i++) {
            if (ans_map <= sum)
                return (ans_map - last_sum) + window[i - 1][0];

            last_sum = sum;
            sum += window[i][1] - window[i][0];
        }
        return 0;
    }
}; //! class Timestamp

} //! namespace fastsgg
} //! namespace gl