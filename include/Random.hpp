/**
 * create time : 2020-10-09
 * Random class
 */
#pragma once

#include <random>
#include <chrono>
#include <cstdlib>
#include "types.hpp"

namespace gl {
namespace fastsgg {

class Random
{
private:
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution;

public:
    Random() {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        generator = std::default_random_engine(seed);
    }

    ~Random() {}

    double nextReal() {
        return distribution(generator);
    }

    double nextReal(double bound) { // [0, bound]
        std::uniform_real_distribution<double> uni_db(0, bound);
        return uni_db(generator);
    }

    int nextInt32(int bound) { // [0, bound]
        // std::default_random_engine gen;
        // std::uniform_int_distribution<int> uni_int(0, bound);
        // return uni_int(gen);
        return (rand() % (bound + 1));
    }

    int_t nextInt(int_t bound) { // [0, bound]
        std::uniform_int_distribution<int_t> uni_int(0, bound);
        return uni_int(generator);
    }

    std::vector<int_t> nextInts(const std::vector<int_t>& w, int_t c, bool chosen) {
        std::discrete_distribution<int_t> dis_int(w.begin(), w.end());
        if (chosen) {
            std::vector<int_t> ans(c);
            for (int i = 0; i < c; i++) { ans[i] = dis_int(generator); }
            return ans;
        } else {
            std::vector<int_t> ans(w.size());
            for (int i = 0; i < c; i++) { ans[dis_int(generator)]++; }
            return ans;
        }
    }
}; //! class Random

} //! namespace fastsgg
} //! namespace gl