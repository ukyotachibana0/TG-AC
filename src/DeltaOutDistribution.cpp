/**
 * create time : 2020-11-10
 * Implementation of DeltaOutDistribution
 */
#include "DeltaOutDistribution.hpp"

namespace gl {
namespace fastsgg {

DeltaOutDistribution::DeltaOutDistribution(OutDegreeDistribution *odd1,
    OutDegreeDistribution *odd2) {
    distr1 = odd1;
    distr2 = odd2;
    if (odd2 == nullptr) {
        std::cerr << "[DeltaOutDistribution::DeltaOutDistribution] The 2nd parameter must not be null" << std::endl;
        return;
    }

    min_degree = odd2->get_min_degree();
    num_nodes = odd2->get_nodes();
    bucket = 2;
    bucket_step = 2.0 / num_nodes;
    vernier_caliper();
    build_cdf();
}

void DeltaOutDistribution::set_bucket(int_t b) {
    bucket = b;
    bucket_step = b * 1.0 / num_nodes;
}

void DeltaOutDistribution::vernier_caliper() {
    if (distr1 == nullptr) {
        range = distr2->get_degree_range();
        std::vector<int_t> res = distr2->get_each_degree_nums();
        each_degree_nums.resize(range);
        each_degree_list.resize(range);
        for (int_t i = 0; i < range; ++i) {
            each_degree_nums[i] = res[i];
            each_degree_list[i] = i + min_degree;
        }
        return;
    }
    // critical part
    int_t odd1_range = distr1->get_degree_range();
    int_t odd2_range = distr2->get_degree_range();
    int_t odd1_max_degree = odd1_range + min_degree - 1;
    int_t odd2_max_degree = odd2_range + min_degree - 1;
    std::vector<int_t> odd1_degree_nums = distr1->get_each_degree_nums();
    std::vector<int_t> odd2_degree_nums = distr2->get_each_degree_nums();
    // split to achieve alignment
    int_t d1 = odd1_max_degree;
    int_t d2 = odd2_max_degree;
    int_t i = odd1_degree_nums.size() - 1;
    int_t j = odd2_degree_nums.size() - 1;
    std::vector<int_t> degree_list;
    std::vector<int_t> number_list;
    while (i >= 0 && j >= 0) {
        if (odd1_degree_nums[i] > odd2_degree_nums[j]) {
            degree_list.push_back(d2 - d1);
            number_list.push_back(odd2_degree_nums[j]);
            odd1_degree_nums[i] -= odd2_degree_nums[j];
            j --; d2 --;
        } else if (odd1_degree_nums[i] == odd2_degree_nums[j]) {
            degree_list.push_back(d2 - d1);
            number_list.push_back(odd2_degree_nums[j]);
            i --; d1 --;
            j --; d2 --;
        } else {
            degree_list.push_back(d1 - d1);
            number_list.push_back(odd1_degree_nums[i]);
            i --; d1 --;
        }
    }
    while (j >= 0) {
        degree_list.push_back(d2);
        number_list.push_back(odd2_degree_nums[j]);
        j --; d2 --;
    }
    // end alignment

    // merge switch
    int_t len = degree_list.size();
    for (i = j = 0; i < len; ) {
        int_t d = degree_list[i];
        while (i < len && d == degree_list[i]) i ++;
        j ++;
    }
    each_degree_nums.resize(j);
    each_degree_list.resize(j);
    int_t k = 0;
    for (i = 0; i < len; ) {
        int_t d = degree_list[i];
        each_degree_nums[k] = 0;
        each_degree_list[k] = d;
        while (i < len && d == degree_list[i]) {
            each_degree_nums[k] += number_list[i];
            i ++;
        }
        k ++;
    }
    // end merging

    // reverse
    i = 0;
    j = each_degree_nums.size() - 1;
    while (i < j) {
        std::swap(each_degree_nums[i], each_degree_nums[j]);
        std::swap(each_degree_list[i], each_degree_list[j]);
        i++, j--;
    }

    range = each_degree_nums.size();
}

void DeltaOutDistribution::build_cdf() {
    int_t sum = 0, cum = 0;
    for (int_t i = 0; i < range; ++i)
        sum += each_degree_nums[i];
    std::vector<double> cdf(range);
    double pre = 0.0;
    min_gap = 1.0;
    for (int_t i = 0; i < range; ++i) {
        cum += each_degree_nums[i];
        cdf[i] = 1.0 * cum / sum;
        min_gap = std::min(min_gap, cdf[i] - pre);
        pre = cdf[i];
    }

    int_t length = Utility::mathCeil(1.0 / min_gap) + 1;
    for_operating.resize(length);
    pre = 0.0;
    int_t j = 0;
    for (int_t i = 0; i < range; ++i) {
        int_t steps = (int_t)((cdf[i] - pre) / min_gap);
        for (int_t p = 0; p < steps; ++p)
            for_operating[j ++] = each_degree_list[i];
        pre += steps * min_gap;
        while (pre < cdf[i]) {
            for_operating[j ++] = each_degree_list[i];
            pre += min_gap;
        }
    }
    while (j < length)
        for_operating[j ++] = each_degree_list[range - 1];
}

int_t DeltaOutDistribution::hF(double x) {
    return std::min((int_t)Utility::mathRound(x / min_gap), (int_t)(for_operating.size() - 1));
}

int_t DeltaOutDistribution::get_out_degree(int_t id) {
    double rv = ((id / bucket) + rand.nextReal()) * bucket_step;
    int_t index = (int_t)std::min(hF(rv), (int_t)(for_operating.size() - 1));
    return for_operating[index];
}

void DeltaOutDistribution::show_degree_nums() {
    std::cout << "[DeltaOutDistribution::show_degree_nums] : " << std::endl;
    int_t length = each_degree_nums.size();
    for (int_t i = 0; i < length; ++i)
        std::cout << each_degree_nums[i] << " ";
    std::cout << std::endl;
    for (int_t i = 0; i < length; ++i)
        std::cout << each_degree_list[i] << " ";
    std::cout << std::endl;
}

} //! namespace fastsgg
} //! namespace gl
