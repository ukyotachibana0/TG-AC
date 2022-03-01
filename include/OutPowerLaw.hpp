/**
 * create time : 2020-11-11
 * OutPowerLaw header
 */
#pragma once

#include "OutDegreeDistribution.hpp"

namespace gl {
namespace fastsgg {
 
class OutPowerLaw : public OutDegreeDistribution
{
public:
    OutPowerLaw(int_t mid, int_t mxd, int_t n, int_t m, std::unordered_map<std::string, double>& params) : OutDegreeDistribution(mid, mxd, n, m, params) {}

    double pdf(int_t x) {
        double lambda = theta["lambda"];
        return pow((double)(x), lambda);
    }
}; //! class OutPowerLaw

} //! namespace fastsgg
} //! namespace gl