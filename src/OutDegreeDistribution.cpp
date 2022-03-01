/**
 * create time : 2020-11-09
 * Implementation of OutDegreeDistribution
 */
#include "OutDegreeDistribution.hpp"

namespace gl {
namespace fastsgg {

OutDegreeDistribution::OutDegreeDistribution(int_t min_d, int_t max_d, int_t n, int_t m,
    std::unordered_map<std::string, double>& params) {
    min_degree = min_d;
    max_degree = max_d;
    num_nodes = n;
    num_edges = m;
    // for power-law
    if (params.count("lambda"))
        double lambda = -fabs(params["lambda"]);
    theta = params;
    // match_edges();
    // build();
}

void OutDegreeDistribution::pre_propcess() {
    match_edges();
    build();
}

int_t OutDegreeDistribution::get_nodes() {
    return num_nodes;
}

int_t OutDegreeDistribution::get_min_degree() {
    return min_degree;
}

int_t OutDegreeDistribution::get_max_degree() {
    return max_degree;
}

int_t OutDegreeDistribution::get_degree_range() {
    return degree_range;
}

std::vector<int_t> OutDegreeDistribution::get_each_degree_nums() {
    return each_degree_nums;
}

int_t OutDegreeDistribution::number_of_max_degree() {
    double max_pdf = pdf(max_degree);
    double sum_pdf = max_pdf;
    for (int_t i = min_degree; i < max_degree; ++i)
        sum_pdf += pdf(i);
    return Utility::mathRound(num_nodes * max_pdf / sum_pdf);
}

int_t OutDegreeDistribution::current_edges() {
    degree_range = max_degree - min_degree + 1;
    double sum = 0.0;
    std::vector<double> p(degree_range);
    for (int_t i = min_degree; i <= max_degree; ++i) {
        p[i - min_degree] = pdf(i);
        sum += p[i - (int_t)(min_degree)];
    }
    double alpha = num_nodes / sum;
    int_t ans = 0;
    for (int_t i = 0; i < degree_range; ++i)
        ans += Utility::mathRound(alpha * p[i] * (i + min_degree));
    return ans;
}

void OutDegreeDistribution::match_edges() {
    max_degree = std::min(max_degree, (int_t)num_nodes);
    int_t bin_l = max_degree + 1;
    int_t bin_r = max_degree;
    while (bin_r > 0 && number_of_max_degree() < 1) {
        bin_r = max_degree;
        max_degree /= 2;
        bin_l = max_degree;
    }
    while (bin_l < bin_r) {
        max_degree = bin_l + (bin_r - bin_l) / 2;
        int_t num = number_of_max_degree();
        if (num < 1)
            bin_r = max_degree;
        else
            bin_l = max_degree + 1;
    }
    max_degree = bin_l - 1;

    int_t actual_edges = current_edges();

#ifdef DEBUG
    std::cout << "[OutDegreeDistribution::match_edges] Adjust max_degree" << std::endl;
    std::cout << "[OutDegreeDistribution::match_edges] Max degree = " << max_degree << std::endl;
    std::cout << "[OutDegreeDistribution::match_edges] Actual edges = " << actual_edges << std::endl;
    std::cout << "[OutDegreeDistribution::match_edges] Expected edges = " << num_edges << std::endl;
#endif

    if (actual_edges < num_edges) {
        int_t temp = max_degree;
        bin_l = bin_r = max_degree + 1;
        while (max_degree < num_nodes && number_of_max_degree() > 0) {
            bin_l = max_degree;
            max_degree *= 2;
            bin_r = max_degree;
        }
        bin_r = std::min(bin_r, (int_t)num_nodes);
        while (bin_l < bin_r) {
            max_degree = bin_l + (bin_r - bin_l) / 2;
            if (number_of_max_degree() < 1)
                bin_r = max_degree;
            else
                bin_l = max_degree + 1;
        }
        max_degree = bin_l - 1;
        actual_edges = current_edges();
        if (actual_edges > num_edges) {
            bin_l = temp;
            bin_r = max_degree;
            while (bin_l < bin_r) {
                max_degree = bin_l + (bin_r - bin_l) / 2;
                actual_edges = current_edges();
                if (actual_edges > num_edges)
                    bin_r = max_degree;
                else
                    bin_l = max_degree + 1;
            }
            max_degree = bin_l - 1;
            actual_edges = current_edges();
        }
    } else if (actual_edges > num_edges) {
        bin_l = min_degree;
        bin_r = max_degree;
        while (bin_l < bin_r) {
            max_degree = bin_l + (bin_r - bin_l) / 2;
            actual_edges = current_edges();
            if (actual_edges > num_edges)
                bin_r = max_degree;
            else
                bin_l = max_degree + 1;
        }
        max_degree = bin_l - 1;
        actual_edges = current_edges();
    }

#ifdef DEBUG
    std::cout << "[OutDegreeDistribution::match_edges] Max degree = " << max_degree << std::endl;
    std::cout << "[OutDegreeDistribution::match_edges] Actual edges = " << actual_edges << std::endl;
    std::cout << "[OutDegreeDistribution::match_edges] Expected edges = " << num_edges << std::endl;
#endif

    degree_range = max_degree - min_degree + 1;
}

void OutDegreeDistribution::build() {
    double sum = 0.0;
    std::vector<double> temp(degree_range);
    for (int_t i = min_degree; i <= max_degree; ++i) {
        temp[i - min_degree] = pdf(i);
        sum += temp[i - min_degree];
    }
    double alpha = num_nodes / sum;
    each_degree_nums.resize(degree_range);
    for (int_t i = 0; i < degree_range; ++i)
        each_degree_nums[i] = (int_t)(Utility::mathRound(alpha * temp[i]));
}

void OutDegreeDistribution::show_degree_nums() {
    std::cout << "[OutDegreeDistribution::show_degree_nums] ..." << std::endl;
    size_t length = each_degree_nums.size();
    for (size_t i = 0; i < length; ++i)
        std::cout << each_degree_nums[i] << " ";
    std::cout << std::endl;
    for (size_t i = 0; i < length; ++i)
        std::cout << i + min_degree << " ";
    std::cout << std::endl;
}

} //! namespace fastsgg
} //! namespace gl
