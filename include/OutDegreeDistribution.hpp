/**
 * create time : 2020-11-09
 * OutDegreeDistribution header
 */
#pragma once

#include "headers.hpp"

namespace gl {
namespace fastsgg {

class OutDegreeDistribution {
private:
    int_t min_degree;
    int_t max_degree;
    int_t degree_range;
    int_t num_nodes;
    int_t num_edges;
    std::vector<int_t> each_degree_nums;

protected:
    std::unordered_map<std::string, double> theta;

public:
    OutDegreeDistribution(int_t min_d, int_t max_d, int_t n, int_t m, std::unordered_map<std::string, double>& params);

    void pre_propcess();

    int_t get_nodes();

    int_t get_min_degree();

    int_t get_max_degree();

    int_t get_degree_range();

    std::vector<int_t> get_each_degree_nums();

    // override
    virtual double pdf(int_t x) = 0;

    int_t number_of_max_degree();

    int_t current_edges();

    void match_edges();

    void build();

    void show_degree_nums();

}; //! class OutDegreeDistribution

} //! namespace fastsgg
} //! namespace gl