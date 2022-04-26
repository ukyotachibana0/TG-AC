/**
 * create time : 2020-10-10
 * Utility
 */
#include "Utility.hpp"
#include "headers.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace gl {
namespace fastsgg {

double Utility::mathCeil(double x) {
    double y = (double)((int)x);
    if (y == x) return x;
    if (y >= 0.0) return (y + 1.0);
    else return (y - 1.0);
}

double Utility::mathRound(double x) {
    double y = (double)((int)x);
    double delta = x - y;
    if (delta > 0.5) return (y + 1.0);
    else if (delta >= -0.5 && delta <= 0.5) return y;
    else return y - 1.0;
}

double Utility::normPdf(double x) {
    return exp(-x * x / 2.0) / sqrt(2.0 * PI);
}

double Utility::normCdf(double x) {
    if (x < -8.0) return 0.0;
    if (x > 8.0) return 1.0;
    double sum = 0.0, term = x;
    for (int i = 3; sum + term != sum; i += 2) {
        sum += term;
        term *= x * x / i;
    }
    return 0.5 + sum * normPdf(x);
}

void Utility::showList(const std::vector<int_t>& list) {
    for (auto n : list)
        std::cout << n << " ";
    std::cout << std::endl;
}

std::vector<int_t> Utility::splitDegree(const std::vector<std::vector<int_t>>& comm_split, int_t c, bool pos) {
    int n = comm_split.size();
    std::vector<int_t> w(n);
    for (int i = 0; i < n; i++) { w[i] = comm_split[i][pos]; }
    Random rand;
    return rand.nextInts(w, c);
}

std::vector<int_t> Utility::splitScalar(int_t n, int_t k, double lambda) {
    lambda = -fabs(lambda);
    int_t avg_size = n / k;
    int_t step = 0;
    int_t memory = 0;
    int_t lr = 0;
    int_t mem_count = 0;
    std::vector<int_t> al_size(1, 0);
    std::vector<int_t> al_count(1, 0);
    while (true) {
        int_t curr_size = 1 + lr * 2;
        std::vector<int_t> size_list(curr_size);
        size_list[lr] = avg_size;
        step = std::max(avg_size / (lr + k + 2.0), 1.0);
        for (int_t i = 1; i <= lr; ++i) {
            size_list[lr + i] = size_list[lr + i - 1] + step;
            size_list[lr - i] = size_list[lr - i + 1] - step;
        }
        std::vector<double> probability(curr_size);
        for (int_t i = 0; i < curr_size; ++i)
            probability[i] = pow((double)(size_list[i]), lambda);
        double last_p = probability[curr_size - 1];
        std::vector<int_t> count(curr_size);
        int_t spectrum = 0;
        for (int_t i = 0; i < curr_size; ++i) {
            if (std::isnan(probability[i]) || std::isinf(probability[i])) {
                count[i] = 0;
                continue;
            }
            count[i] = (int_t)mathRound(probability[i] / last_p);

#ifdef DEBUG
            if (count[i] == -0x7fffffff || count[i] == 0x80000000) {
                std::cout << "avg_size = " << avg_size << std::endl;
                std::cout << "lr = " << lr << ", step = " << step << std::endl;
                std::cout << "i = " << i << ", curr_size = " << curr_size << std::endl;
                std::cout << "size_list[i] = " << size_list[i] << std::endl;
                std::cout << "probability[i] = " << probability[i] << std::endl;
                std::cout << "last_p = " << last_p << std::endl;
                exit(0);
            }
#endif

            spectrum += count[i] * size_list[i];
        }
        if (spectrum > n) break;
        memory = spectrum;
        mem_count = 0;
        for (int_t i = 0; i < curr_size; ++i) {
            mem_count += count[i];
            if (i >= al_size.size()) {
                al_size.push_back(size_list[i]);
                al_count.push_back(count[i]);
            } else {
                al_size[i] = size_list[i];
                al_count[i] = count[i];
            }
        }
        lr += 1;
    }
    al_size.back() += n - memory;

#ifdef DEBUG
    std::cout << "mem_count = " << mem_count << std::endl;
#endif

    mem_count = 0;
    for (int_t i = 0; i < al_count.size(); ++i)
        mem_count += al_count[i];

#ifdef DEBUG
    std::cout << "mem_count = " << mem_count << std::endl;
#endif

    std::vector<int_t> res(mem_count + 2);
    int_t j = 0;
    for (int_t i = al_size.size() - 1; i >= 0; --i) {
        for (int_t p = 0; p < al_count[i]; ++p) {
            if (j < res.size())
                res[j ++] = al_size[i];
        }
    }

    std::vector<int_t> ans(k);
    if (mem_count < k) {
        int_t fission = k / mem_count + ((k % mem_count == 0) ? 0 : 1);
        int_t more = k - mem_count;
        int_t res_i = 0, ans_i = 0;
        while (more > 0) {
            int_t cumu_size = 0;
            int_t per_size = res[res_i] / fission;
            for (int_t i = 0; i < fission - 1; ++i) {
                ans[ans_i] = per_size;
                ans_i ++;
                cumu_size += per_size;
            }
            ans[ans_i] = res[res_i] - cumu_size;
            ans_i ++;
            res_i ++;
            more -= (fission - 1);
        }
        while (ans_i < k && res_i < mem_count) {
            ans[ans_i] = res[res_i];
            ans_i ++;
            res_i ++;
        }
    } else if (mem_count > k) {
        int_t fission = mem_count / k + ((mem_count % k == 0) ? 0 : 1);
        int_t less = mem_count - k;
        int_t res_i = mem_count - 1;
        int_t ans_i = k - 1;
        while (less > 0) {
            ans[ans_i] = res[res_i];
            res_i --;
            for (int_t i = 0; i < fission - 1; ++i) {
                ans[ans_i] += res[res_i];
                res_i --;
            }
            less -= (fission - 1);
            ans_i --;
        }
        while (ans_i >= 0 && res_i >= 0) {
            ans[ans_i] = res[res_i];
            ans_i --;
            res_i --;
        }
    } else {
        std::copy_n(res.begin(), k, ans.begin());
    }
    return ans;
}

std::vector<std::vector<int_t>> Utility::splitCommunity(
    int_t row, int_t col, int_t k, double lambda) {
    std::vector<int_t> row_ans = splitScalar(row, k, lambda);

#ifdef DEBUG
    std::cout << "[Utility::splitCommunity] Row size = " << row_ans.size() << std::endl;
#endif

    std::vector<std::vector<int_t>> ans(row_ans.size(), std::vector<int_t>(2));
    if (row == col) {
        for (size_t i = 0; i < row_ans.size(); ++i)
            ans[i][0] = ans[i][1] = row_ans[i];
    } else {
        int_t cum = 0;
        for (int_t i = row_ans.size() - 1; i > 0; --i) {
            ans[i][0] = row_ans[i];
            ans[i][1] = (int_t)(mathRound(row_ans[i] * 1.0 * col / row * 1.0));
            cum += ans[i][1];
        }
        ans[0][0] = row_ans[0];
        ans[0][1] = col - cum;
    }
    return ans;
}

std::vector<std::vector<int_t>> Utility::splitWindow(int_t n, int_t sz, int_t min_ts, int_t max_ts) {
    std::vector<std::vector<int_t>> ans(n, std::vector<int_t>({min_ts, max_ts}));
    if (n == 1) return ans;

    int_t total_ol = (max_ts - min_ts) - n * sz;
    if (total_ol == 0) return ans;

    // equidistant(except at the ending part) and of equal length
    double ol = total_ol / (1.0 * (n - 1)), ts = min_ts;
    for (int_t i = 0; i < n - 1; ++i) {
        ans[i][0] = mathRound(ts);
        ans[i][1] = mathRound(ts + sz);
        ts += sz + ol;
    }
    ans[n - 1][0] = max_ts - sz;

    // adjust right endpoints of each intervals(except the last one)
    Random rand;
    const int_t padding = sz / 10;
    for (int_t i = 0; i < n - 1; i++) {
        int_t right_min = (i ? std::max(ans[i - 1][1], ans[i][0]) : ans[i][0]) + padding;
        int_t right_max = ans[i + 1][1] - padding;
        ans[i][1] = rand.nextInt(right_max - right_min) + right_min;
    }

    /*int_t sz_avg = 0;
    std::cout << "[Utility::splitWindow] ans: ";
    for (auto v : ans) {
        std::cout << "[" << v[0] << "," << v[1] << "](" << v[1] - v[0] + 1 << ") ";
        sz_avg += v[1] - v[0] + 1;
    }
    std::cout << std::endl << "\taverage size: " << sz_avg / (1.0 * n) << std::endl;*/

    return ans;
}

std::vector<std::vector<int_t>> Utility::unionWindow(const std::vector<std::vector<int_t>>& window) {
    std::vector<std::vector<int_t>> ans;
    int_t n = window.size(), i = 0;
    while (i < n - 1) {
        int_t left = window[i][0], right = window[i][1], j = i;
        while (j < n - 1 && window[j + 1][0] <= window[j][1]) { 
            right = window[++j][1];
        }
        ans.push_back({left, right});
        i = j + 1;
    }
    if (i == n - 1) ans.push_back(window.back());

    /*std::cout << "[Utility::unionWindow] ans: ";
    for (auto v : ans) {
        std::cout << "[" << v[0] << "," << v[1] << "]  ";
    }
    std::cout << std::endl;*/

    return ans;
}

std::vector<std::vector<int_t>> Utility::compleWindow(const std::vector<std::vector<int_t>>& window,
    int_t e_mit, int_t e_mat) {
    std::vector<std::vector<int_t>> ans;
    if (e_mit < window.front()[0]) ans.push_back({e_mit, window.front()[0]});
    int n = window.size();
    for (int i = 0; i < n - 1; ++i) { ans.push_back({window[i][1], window[i + 1][0]}); }
    if (e_mat > window.back()[1]) ans.push_back({window.back()[1], e_mat});

    /*std::cout << "[Utility::compleWindow] ans: ";
    for (auto v : ans) {
        std::cout << "[" << v[0] << "," << v[1] << "]  ";
    }
    std::cout << std::endl;*/

    return ans;
}


std::vector<std::unordered_map<int_t, double>> Utility::idenOlAnchorComm(int_t n) {
    const double ol_min = 0.1, ol_max = 0.3;
    const int_t n_pair = n / 2; // every community gets a overlapping anchor community on average

    std::vector<std::unordered_map<int_t, double>> ans(n);
    int_t i = 0;
    Random rand;
    while (i < n_pair) {
       std::pair<int_t, int_t> one_pair(rand.nextInt(n - 1), rand.nextInt(n - 1));
       if (one_pair.first == one_pair.second) continue;
       double ol = rand.nextReal(ol_max - ol_min) + ol_min;
       if (ans[one_pair.first].insert({one_pair.second, 1 - ol}).second &&
           ans[one_pair.second].insert({one_pair.first, 1 - ol}).second) 
           ++i;
    }
    /*double ol = 0.9;
    ans[0].insert({ 1,ol });
    ans[1].insert({ 0,ol });*/

    /*std::cout << "[Utility::idenOlAnchorComm] ans:" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\t" << i << ": ";
        for (auto p : ans[i]) {
            std::cout << "(" << p.first << "," << p.second << ")  ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;*/

    return ans;
}


int Utility::numOneBitInt(uint32_t x) {
    int ans = 0;
    while (x) {
        ans ++;
        x &= (x - 1);
    }
    return ans;
}

std::string Utility::strSeqNumber(int x, int n_bits) {
    // char *buffer = new char[n_bits + 1];
    char buffer[32];
    std::string format = "%0" + std::to_string(n_bits) + "d";
    int res = snprintf(buffer, sizeof(buffer), format.c_str(), x);
    std::string ans = "";
    if (res >= 0 && res < n_bits + 1)
        ans = buffer;
    return ans;
}

void Utility::randomChoice(std::vector<int>& nums, int K) {
    int N = nums.size();
    if (K >= N) {
        return;
    }
    for (int i = 0; i < K; ++i) {
        int x = (rand() % N) + i;
        std::swap(nums[i], nums[x]);
        N --;
    }
}

void Utility::randomChoice(std::vector<int>& nums, std::vector<int>& accomp, int K) {
    int N = nums.size();
    if (K >= N) {
        return;
    }
    for (int i = 0; i < K; ++i) {
        int x = (rand() % N) + i;
        std::swap(nums[i], nums[x]);
        std::swap(accomp[i], accomp[x]);
        N --;
    }
}

ProgressBar::ProgressBar() {
    mi_bar_width = 70;
    md_progress = 0.0;
}

ProgressBar::~ProgressBar() {

}

void ProgressBar::set_progress(double pg) {
    md_progress = pg;
    show();
}

void ProgressBar::show() {
    md_progress = std::min(md_progress, 1.0);
    std::cout << "[";
    int pos = mi_bar_width * md_progress;
    for (int i = 0; i < pos; ++i)
        std::cout << "=";
    std::cout << ">";
    for (int i = pos + 1; i < mi_bar_width; ++i)
        std::cout << " ";
    std::cout << "] " << (int)(md_progress * 100.0) << " %\r";
    std::cout.flush();
}

} //! namespace fastsgg
} //! namespace gl