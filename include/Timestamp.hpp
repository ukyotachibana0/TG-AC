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
}; //! class Timestamp

} //! namespace fastsgg
} //! namespace gl