/**
 * create time : 2020-10-13
 * Normal Distribution
 */
#pragma once

#include "Distribution.hpp"

namespace gl {
namespace fastsgg {

class Normal : public Distribution {
public:
    Normal() : Distribution() {}
    Normal(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params) :
    Distribution(mid, mxd, n, m, params) {}

    double pdf(int_t x) {
        double mu = theta["mu"];
        double sigma = theta["sigma"];
        double a = (x + 0.1 - mu) / sigma;
        double b = (x - 0.1 - mu) / sigma;
        return Utility::normCdf(a) - Utility::normCdf(b);
    }
}; //! class Normal 

} //! namespace fastsgg
} //! namespace gl