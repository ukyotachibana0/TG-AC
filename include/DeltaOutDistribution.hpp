/**
 * create time : 2020-11-09
 * DeltaOutDistribution headers
 */
#pragma once

#include "headers.hpp"
#include "OutDegreeDistribution.hpp"

namespace gl {
namespace fastsgg {

class DeltaOutDistribution {
private:
    OutDegreeDistribution *distr1;
    OutDegreeDistribution *distr2;

    int_t min_degree;
    int_t range;
    int_t num_nodes;
    double min_gap;

    std::vector<int_t> each_degree_nums;
    std::vector<int_t> each_degree_list;
    std::vector<int_t> for_operating;

    Random rand;
    int_t bucket;
    double bucket_step;

public:
    DeltaOutDistribution() : distr1(nullptr), distr2(nullptr) {};

    DeltaOutDistribution(OutDegreeDistribution *odd1, OutDegreeDistribution *odd2);

    void set_bucket(int_t b);

    void vernier_caliper();

    void build_cdf();

    int_t hF(double x);

    int_t get_out_degree(int_t id);

    void show_degree_nums();

}; //! class DeltaOutDistribution

} //! namespace fastsgg
} //! namespace gl