/**
 * create time : 2020-10-09
 * Utility functions
 */
#pragma once
#include <vector>
#include <string>
#include "types.hpp"

namespace gl {
namespace fastsgg {

class Utility
{
public:
    static double mathCeil(double x);

    static double mathRound(double x);

    static double normPdf(double x);

    static double normCdf(double x);

    static void showList(const std::vector<int_t>& list);

    static std::vector<int_t> splitScalar(int_t n, int_t k, double lambda);

    static std::vector<std::vector<int_t>> splitCommunity(int_t row, int_t col, int_t k, double lambda);

    static int numOneBitInt(uint32_t x);

    static std::string strSeqNumber(int x, int n_bits=5);

    static void randomChoice(std::vector<int>& nums, int K);

    static void randomChoice(std::vector<int>& nums, std::vector<int>& accomp, int K);

}; //! class Utility

class ProgressBar
{
public:
    ProgressBar();
    ~ProgressBar();

    void set_progress(double pg);

private:
    int mi_bar_width;
    double md_progress;

    void show();

}; //! class ProgressBar

} //! namespace fastsgg
} //! namespace gl