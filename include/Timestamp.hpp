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

    // int_t genTimestampNum () {
    //     // TODO
    //     return 1; // 1 or 2
    // }

    int_t genTimestamp () {
        int_t bound = max_timestamp - min_timestamp;
        return min_timestamp + rand.nextInt(bound);
    }

    int_t genTimestamp (int_t mit, int_t mat) {
        int_t start = std::max(min_timestamp, mit);
        int_t end = std::min(max_timestamp, mat);
        return start + rand.nextInt(end - start);
    }

    int_t genTimestamp (const std::vector<std::vector<int_t>>& window) {
        /*std::cout << "[Timestamp::genTimestamp] window: ";
        for (auto& w : window) { std::cout << "[" << w[0] << "," << w[1] << "] "; }
        std::cout << std::endl;*/

        int_t ans = 0;
        // gen mapped ans
        int_t bound = 0, n = window.size();
        for (auto& win : window) { bound += win[1] - win[0]; }
        int_t ans_map = rand.nextInt(bound);
        //std::cout << "[Timestamp::genTimestamp] bound: " << bound << " ans_map: " << ans_map << std::endl;
        // gen original ans
        int_t last_sum = 0, sum = window[0][1] - window[0][0];
        for (int_t i = 1; i < n; i++) {
            if (ans_map <= sum) return (ans_map - last_sum) + window[i - 1][0];

            last_sum = sum;
            sum += window[i][1] - window[i][0];
        }
        return (ans_map - last_sum) + window[n - 1][0];
    }
}; //! class Timestamp

} //! namespace fastsgg
} //! namespace gl