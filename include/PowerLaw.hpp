/**
 * create time : 2020-10-13
 * PowerLaw Distribution
 */
#pragma once

#include "Distribution.hpp"

namespace gl {
namespace fastsgg {

class PowerLaw : public Distribution {
public:
    PowerLaw() : Distribution() {}
    PowerLaw(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params) :
    Distribution(mid, mxd, n, m, params) {}

    double pdf(int_t x) {
        double lambda = theta["lambda"];
        return pow((double)(x), lambda);
    }
}; //! class PowerLaw 

} //! namespace fastsgg
} //! namespace gl