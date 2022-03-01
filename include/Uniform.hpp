/**
 * create time : 2020-10-13
 * Uniform Distribution
 */
#pragma once

#include "Distribution.hpp"

namespace gl {
namespace fastsgg {

class Uniform : public Distribution {
private:
    bool is_special;
    int_t num_nodes;
    int_t num_edges;
    int_t min_degree;
    int_t max_degree;
    int_t frequency;
    int_t cur_degree;
    int_t cum_degree;
    int_t cum_frequency;
    int_t cur_id;

    int_t base_id;

    Random rand;

public:
    Uniform() : Distribution() {}

    Uniform(int_t mid, int_t mxd, int_t n, int_t m,
    std::unordered_map<std::string, double>& params) {
        num_nodes = n;
        num_edges = m;

        int_t avg_degre = m / n + 1;
        if (avg_degre <= mid) {
            mid = 1;
            mxd = 2 * avg_degre - mid;
        } else {
            mxd = 2 * avg_degre - mid;
        }
        min_degree = mid;
        max_degree = mxd;
        is_special = (mid == mxd);
        frequency = n / (mxd - mid + 1);
        cur_degree = mid;
        cum_degree = 0;
        cum_frequency = 0;
        cur_id = 0;
        base_id = 0;
    }

    // no used
    double pdf(int_t x) {
        return 0.0;
    }

    virtual int_t genOutDegree(int_t id) {
        if (is_special)
            return min_degree;
        int_t bound = max_degree - min_degree;
        return min_degree + rand.nextInt(bound);
    }

    virtual int_t genTargetID() {
        int_t ans = cur_id;
        cur_id += frequency;
        if (cur_id >= num_nodes) {
            cur_id = ++base_id;
            if (base_id + 1 == num_nodes) {
                base_id = 0;
            }
        }
        return ans;
    }

    virtual int_t _genTargetID_1() {
        // pre-process
        int_t len_range = max_degree - min_degree + 1;
        std::vector<int_t> pos(len_range, 0);
        int_t acc_freq = 0;
        for (int_t i = 1; i < len_range; ++i) {
            pos[i] = pos[i - 1] + (min_degree + i - 1) * frequency;
            acc_freq += frequency;
        }
        int_t bound = pos[len_range - 1] + (max_degree) * (num_nodes - acc_freq);
        // generate
        int_t rand_val = rand.nextInt(bound);
        auto lit = std::lower_bound(begin(pos), end(pos), rand_val);
        int_t idx = lit - pos.begin();
        if (idx == 0) {
            return 0;
        }
        int_t ans = (rand_val - pos[idx - 1]) / (min_degree + idx - 1) + (idx - 1) * frequency;
        return ans;
    }

    virtual int_t _genTargetID_0() {
        int_t ans = cur_id;
        cum_degree ++;
        if (cum_degree == cur_degree) {
            cum_degree = 0;
            cum_frequency ++;
            cur_id ++;
            if (cur_id == num_nodes) {
                // cur_id = num_nodes - 1;
                cur_id = 0;
                cum_degree = 0;
                cum_frequency = 0;
                cur_degree = min_degree;
            }
            if (cum_frequency == frequency) {
                cum_frequency = 0;
                cur_degree ++;
            }
        }
        return ans;
    }

}; //! class Uniform 

} //! namespace fastsgg
} //! namespace gl