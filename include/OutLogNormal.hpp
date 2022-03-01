/**
 * create time : 2020-11-11
 * OutLogNormal header
 */
#pragma once

#include "OutDegreeDistribution.hpp"

namespace gl {
namespace fastsgg {

class OutLogNormal : public OutDegreeDistribution
{
public:
    OutLogNormal(int_t mid, int_t mxd, int_t n, int_t m, std::unordered_map<std::string, double>& params) : OutDegreeDistribution(mid, mxd, n, m, params) {}

    double pdf(int_t x) {
        double mu = theta["mu"];
        double sigma = theta["sigma"];
        double a = (log(x + 0.1) - mu) / sigma;
        double b = (log(x - 0.1) - mu) / sigma;
        return Utility::normCdf(a) - Utility::normCdf(b);
    }

}; //! class OutLogNormal

} //! namespace fastsgg
} //! namespace gl