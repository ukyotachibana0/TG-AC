/**
 * create time : 2022-03-08
 * Author : Zheng Shuwen
 * Timestamp hpp
 */
#pragma once

// #include "headers.hpp"
#include "Random.hpp"

namespace gl {
namespace fastsgg {

class Timestamp
{
private:
    const double MIN_STEP = 0.000001;
    // pre process data
    double step;
    std::vector<int_t> CDF_inv;  // CDF^{-1}

protected:
    // basic info data
    int_t ts_range; // translate domain defination of pdf from [ts_min, ts_max] to [1, ts_range]

public:
    // basic info data
    int_t min_timestamp;
    int_t max_timestamp;
    std::unordered_map<std::string, double> params;

    Random rand;

public:
    Timestamp () : min_timestamp(0), max_timestamp(0), ts_range(0), step(1.0) {}

    Timestamp (int_t mit, int_t mat, const std::unordered_map<std::string, double>& ps)
        : min_timestamp(mit), max_timestamp(mat), ts_range(mat - mit + 1), params(ps), step(1.0) {  
        if (params.count("lambda")) params["lambda"] = -fabs(params["lambda"]);
    }

    virtual double pdf(int_t x) = 0;

    void preProcess() {
        int size = ts_range;
        // compute PDF
        std::vector<double> PDF(size);
        double sum = .0;
        for (int i = 0; i < size; i++) { 
            PDF[i] = pdf(i + 1);    // [1, ts_range]
            sum += PDF[i];
        }
        for (int i = 0; i < size; i++) {
            PDF[i] /= sum;
            step = (0 < PDF[i] && PDF[i] < step) ? PDF[i] : step;
        }
        step = (step < MIN_STEP) ? MIN_STEP : step;

        // compute CDF
        std::vector<double> CDF(size);
        std::partial_sum(PDF.begin(), PDF.end(), CDF.begin());
        
        // compute CDF_inv
        int steps = Utility::mathCeil(1.0 / step);
        CDF_inv.resize(steps);
        int_t j = 0;
        for (int i = 0; i < steps; i++) {
            double cur_step = i * step;
            while (j < size && CDF[j] < cur_step) { j++; }
            CDF_inv[i] = j + min_timestamp;
        }
    }

    virtual int_t genTimestamp() {
        double y = rand.nextReal(1.0);
        return CDF_inv[y / step];
    }

    int_t genTimestamp(const std::vector<std::vector<int_t>>& window) {
        int_t ans = 0;
        // gen mapped ans
        int_t bound = 0, n = window.size();
        for (auto& win : window) { bound += win[1] - win[0]; }
        int_t ans_map = rand.nextInt(bound);
        // gen original ans
        int_t last_sum = 0, sum = window[0][1] - window[0][0];
        for (int_t i = 1; i < n; i++) {
            if (ans_map <= sum) return (ans_map - last_sum) + window[i - 1][0];

            last_sum = sum;
            sum += window[i][1] - window[i][0];
        }
        return (ans_map - last_sum) + window[n - 1][0];
    }
}; //! class Timestamp


class TimestampUniform : public Timestamp {
public:
    TimestampUniform() : Timestamp() {}
    TimestampUniform(int_t mit, int_t mat, const std::unordered_map<std::string, double>& ps)
        : Timestamp(mit, mat, ps) {}

    double pdf(int_t x) {
        return .0;
    }

    virtual int_t genTimestamp() {
        int_t bound = max_timestamp - min_timestamp;
        return min_timestamp + rand.nextInt(bound);
    }
};


class TimestampPowerLaw : public Timestamp {
public:
    TimestampPowerLaw() : Timestamp() {}
    TimestampPowerLaw(int_t mit, int_t mat, const std::unordered_map<std::string, double>& ps)
        : Timestamp(mit, mat, ps) {}

    double pdf(int_t x) {
        double lambda = params["lambda"];
        return pow((double)(x), lambda);
    }
};


class TimestampNormal : public Timestamp {
public:
    TimestampNormal() : Timestamp() {}
    TimestampNormal(int_t mit, int_t mat, const std::unordered_map<std::string, double>& ps)
        : Timestamp(mit, mat, ps) {}

    double pdf(int_t x) {
        double mu = params.count("mu") ? params["mu"] : (ts_range >> 1);
        double sigma = params["sigma"];
        double a = (x + 0.1 - mu) / sigma;
        double b = (x - 0.1 - mu) / sigma;
        return Utility::normCdf(a) - Utility::normCdf(b);
    }
};


class TimestampLogNormal : public Timestamp {
public:
    TimestampLogNormal() : Timestamp() {}
    TimestampLogNormal(int_t mit, int_t mat, const std::unordered_map<std::string, double>& ps)
        : Timestamp(mit, mat, ps) {}

    double pdf(int_t x) {
        double mu = params.count("mu") ? params["mu"] : (ts_range >> 1);
        double sigma = params["sigma"];
        double a = (log(x + 0.1) - mu) / sigma;
        double b = (log(x - 0.1) - mu) / sigma;
        return Utility::normCdf(a) - Utility::normCdf(b);
    }
};


} //! namespace fastsgg
} //! namespace gl