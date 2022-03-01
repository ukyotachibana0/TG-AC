/**
 * create time : 2020-11-11
 * OutNormal header
 */
#pragma once

#include "OutDegreeDistribution.hpp"

namespace gl {
namespace fastsgg {

class OutNormal : public OutDegreeDistribution
{
public:
    OutNormal(int_t mid, int_t mxd, int_t n, int_t m, std::unordered_map<std::string, double>& params) : OutDegreeDistribution(mid, mxd, n, m, params) {}

    double pdf(int_t x) {
        double mu = theta["mu"];
        double sigma = theta["sigma"];
        double a = (x + 0.1 - mu) / sigma;
        double b = (x - 0.1 - mu) / sigma;
        return Utility::normCdf(a) - Utility::normCdf(b);
    }

}; //! class OutNormal

} //! namespace fastsgg
} //! namespace gl