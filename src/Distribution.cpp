/**
 * create time : 2020-10-11
 * Distribution Implementation
 */
#include "Distribution.hpp"

namespace gl {
namespace fastsgg {

Distribution::Distribution() {
    min_degree = 0;
    max_degree = 0;
    num_nodes = 0;
    num_edges = 0;
    degree_range = 0;
    bucket = 1;
    bucket_step = 1.0 / num_nodes;
    od_simple_cur_no = od_simple_cur_id = od_simple_offset = 0;
}

Distribution::Distribution(int_t mid, int_t mxd, int_t n, int_t m,
    std::unordered_map<std::string, double>& params) {
    min_degree = mid;
    max_degree = mxd;
    if (min_degree < 1) {
        min_degree = 1;
    }
    if (max_degree < 1) {
        max_degree = 1;
    }
    num_nodes = n;
    num_edges = m;
    degree_range = max_degree - min_degree + 1;
    // for power-law distribution
    if (params.count("lambda")) {
        params["lambda"] = -fabs(params["lambda"]);
    }
    theta = params;
    // bucket
    bucket = 1;
    bucket_step = 1.0 / (num_nodes * 1.0);
    od_simple_cur_no = od_simple_cur_id = od_simple_offset = 0;
}

Distribution::~Distribution() {

}

// build auxiliary function
void Distribution::preProcess(bool out) {
    matchEdges();

#ifdef DEBUG
    std::cout << "[Distribution::preProcess] Match edges." << std::endl;
#endif

    if (out) {
        // buildForOdSimply();

#ifdef DEBUG
        std::cout << "[Distribution::preProcess] Before building for out ..." << std::endl;
#endif

        buildForOdIntricately();

#ifdef DEBUG
        std::cout << "[Distribution::preProcess] After building for out ..." << std::endl;
#endif

    } else {

#ifdef DEBUG
        std::cout << "[Distribution::preProcess] Before building for in ..." << std::endl;
#endif

        buildForIdHashTables();

#ifdef DEBUG
        std::cout << "[Distribution::preProcess] After building for in ..." << std::endl;
#endif

    }
}

// set start source number
void Distribution::setStartSourceNo(int_t no) {
    if (no >= num_nodes)
        return;
    od_simple_mem_no = no ++;
    od_simple_mem_id = 0;
    int_t cum = 0;
    while (od_simple_mem_id < od_simple_cnum.size() && cum < no)
        cum += od_simple_cnum[++ od_simple_mem_id];
    od_simple_cur_no = od_simple_mem_no;
    od_simple_cur_id = od_simple_mem_id;
    od_simple_offset = od_simple_mem_off = cum - no;
}

void Distribution::setOutBucket(int x) {
    bucket = x;
    bucket_step = x * 1.0 / num_nodes;
}

// // override
// double Distribution::pdf(int_t x) {
//     return 0.0;
// }

int_t Distribution::numberOfMinDegree() {
    double min_pdf = pdf(min_degree);
    double sum_pdf = min_pdf;
    for (int_t i = min_degree + 1; i <= max_degree; ++i)
        sum_pdf += pdf(i);
    return Utility::mathRound(num_nodes * min_pdf / sum_pdf);
}

int_t Distribution::numberOfMaxDegree() {
    double max_pdf = pdf(max_degree);
    double sum_pdf = max_pdf;
    for (int_t i = min_degree; i < max_degree; ++i)
        sum_pdf += pdf(i);
    return Utility::mathRound(num_nodes * max_pdf / sum_pdf);
}

int_t Distribution::currentEdges() {
    degree_range = max_degree - min_degree + 1;
    double sum = 0.0;
    std::vector<double> p((int)(degree_range));
    for (int_t i = min_degree; i <= max_degree; ++i) {
        p[i - min_degree] = pdf(i);
        sum += p[i - min_degree];
    }
    double alpha = num_nodes * 1.0 / sum;
    int_t ans = 0;
    for (int i = 0; i < degree_range; ++i)
        ans += Utility::mathRound(alpha * p[i] * (i + min_degree));
    return ans;
}

void Distribution::matchEdges() {
    // adjust max_degree so that
    // # nodes(degree = max_degree) >= 1
    max_degree = std::min(max_degree, num_nodes);
    int_t bin_l = max_degree + 1;
    int_t bin_r = max_degree;
    while (bin_r > 0 && numberOfMaxDegree() < 1) {
        bin_r = max_degree;
        max_degree /= 2;
        bin_l = max_degree;
    }
    while (bin_l < bin_r) {
        max_degree = bin_l + (bin_r - bin_l) / 2;
        if (numberOfMaxDegree() < 1)
            bin_r = max_degree;
        else
            bin_l = max_degree + 1;
    }
    max_degree = bin_l - 1;
    // end adjust max_degree

    int_t actual_edges = currentEdges();
#ifdef DEBUG
    std::cout << "[Distribution::matchEdges] Max degree = " << max_degree << std::endl;
    std::cout << "[Distribution::matchEdges] Actual edges = " << actual_edges << std::endl;
    std::cout << "[Distribution::matchEdges] [Distribution::matchEdges] Expected edges = " << num_edges << std::endl;
#endif

    // => expected # edges
    if (actual_edges < num_edges) {
        int_t temp = max_degree;
        bin_l = bin_r = max_degree + 1;
        while (max_degree < num_nodes && numberOfMaxDegree() > 0) {
            bin_l = max_degree;
            max_degree *= 2;
            bin_r = max_degree;
        }
        bin_r = std::min(bin_r, num_nodes);
        while (bin_l < bin_r) {
            max_degree = bin_l + (bin_r - bin_l) / 2;
            if (numberOfMaxDegree() < 1)
                bin_r = max_degree;
            else
                bin_l = max_degree + 1;
        }
        max_degree = bin_l - 1;
        actual_edges = currentEdges();
        if (actual_edges > num_edges) {
            bin_l = temp;
            bin_r = max_degree;
            while (bin_l < bin_r) {
                max_degree = bin_l + (bin_r - bin_l) / 2;
                actual_edges = currentEdges();
                if (actual_edges > num_edges)
                    bin_r = max_degree;
                else
                    bin_l = max_degree + 1;
            }
            max_degree = bin_l - 1;
            actual_edges = currentEdges();
        }
    } else if (actual_edges > num_edges) {
        bin_l = min_degree;
        bin_r = max_degree;
        while (bin_l < bin_r) {
            max_degree = bin_l + (bin_r - bin_l) / 2;
            actual_edges = currentEdges();
            if (actual_edges > num_edges)
                bin_r = max_degree;
            else
                bin_l = max_degree + 1;
        }
        max_degree = bin_l - 1;
        actual_edges = currentEdges();
    }

#ifdef DEBUG
    std::cout << "[Distribution::matchEdges] Adjusted max degree = " << max_degree << std::endl;
    std::cout << "[Distribution::matchEdges] Actual #edges = " << actual_edges << std::endl;
    std::cout << "[Distribution::matchEdges] Expected #edges = " << num_edges << std::endl;
#endif

    degree_range = max_degree - min_degree + 1;
}

void Distribution::buildForOdSimply() {
    int size = (int)degree_range;
    std::vector<double> temp(size);
    double sum = 0.0;
    for (int i = min_degree; i <= max_degree; ++i) {
        temp[i - min_degree] = pdf(i);
        sum += temp[i - min_degree];
    }
    double alpha = num_nodes / sum;
    od_simple_cnum.resize(size);
    for (int i = 0; i < size; ++i)
        od_simple_cnum[i] = Utility::mathRound(alpha * temp[i]);
}

void Distribution::buildForOdIntricately() {
    // get CDF of out-degree distribution
    int size = (int)degree_range;
    std::vector<double> temp(size);
    double sum = 0.0;
    double min_gap = 1.0;
    double pre = 0.0;
    for (int i = min_degree; i <= max_degree; ++i) {
        sum += pdf(i);
        temp[i - min_degree] = sum;
    }
    double alpha = 1.0 / sum;
    for (int i = 0; i < size; ++i) {
        temp[i] *= alpha;
        min_gap = std::min(min_gap, temp[i] - pre);
        pre = temp[i];
    }

    int length = (int)Utility::mathCeil(1.0 / min_gap) + 1;
    od_complex_degree.resize(length);
    pre = 0.0;
    int j = 0;
    int_t d = min_degree;

    // build
    for (int i = 0; i < degree_range; ++i) {
        int steps = (int)((temp[i] - pre) / min_gap);
        for (int p = 0; p < steps; ++p)
            od_complex_degree[j ++] = d;
        pre += steps * min_gap;
        while (pre < temp[i]) {
            od_complex_degree[j ++] = d;
            pre += min_gap;
        }
        d = i + min_degree;
    }
    while (j < length)
        od_complex_degree[j ++] = d;
    o_min_gap = min_gap;
}

void Distribution::buildForIdHashTables() {
    int size = (int)degree_range;
    std::vector<double> temp(size);
    double cum = 0.0;
    for (int i = min_degree; i <= max_degree; ++i) {
        cum += pdf(i);
        temp[i - min_degree] = cum;
    }
    double alpha = num_nodes / cum;
    for (int i = 0; i < degree_range; ++i) {
        temp[i] *= alpha;
    }

    // build CDF
    std::vector<double> i_CDF(size + 1);
    std::vector<int_t> i_index(size + 1);
    i_CDF[0] = 0.0;
    i_index[0] = -1;
    for (int i = 1; i <= degree_range; ++i) {
        int_t num = (int_t)Utility::mathRound(temp[i - 1]);
        i_index[i] = num;
        i_CDF[i] = num * (i - 1 + min_degree);
    }
    i_min_rv = min_degree / i_CDF[size];
    i_CDF[0] = 0.0;
    i_index[0] = 0;
    i_index[size] = num_nodes - 1;

    i_min_gap = 1.0;
    for (int i = 1; i <= degree_range; ++i) {
        i_CDF[i] /= i_CDF[size];
        i_min_gap = std::min(i_min_gap, i_CDF[i] - i_CDF[i - 1]);
    }

    // build hash tables
    int length = (int)Utility::mathCeil(1.0 / i_min_gap) + 1;
    id_hash_tid.resize(length);
    id_hash_cdf.resize(length);
    id_hash_ratio.resize(length);
    int_t i_tid_next;
    int_t pre_v = 0;
    int pre_i = 0;
    double i_cdf_next;
    for (int j = 1; j <= degree_range; ++j) {
        int i = (int)iHF(i_CDF[j]);
        i_tid_next = i_index[j];
        i_cdf_next = i_CDF[j];
        for (int k = pre_i; k < i; ++k) {
            id_hash_tid[k] = pre_v;
            id_hash_cdf[k] = i_CDF[j - 1];
            if ((i_cdf_next - id_hash_cdf[k]) < 1e-8)
                id_hash_ratio[k] = 0.0;
            else
                id_hash_ratio[k] = (i_tid_next - id_hash_tid[k]) / (i_cdf_next - id_hash_cdf[k]);
        }
        pre_v = i_index[j];
        pre_i = i;
    }
    i_tid_next = num_nodes - 1;
    i_cdf_next = 1.0;
    for (int k = pre_i; k < length; ++k) {
        id_hash_tid[k] = pre_v;
        id_hash_cdf[k] = i_CDF[size];
        if ((i_cdf_next - id_hash_cdf[k]) < 1e-8)
                id_hash_ratio[k] = 0.0;
        else
            id_hash_ratio[k] = (i_tid_next - id_hash_tid[k]) / (i_cdf_next - id_hash_cdf[k]);
    }
}

int_t Distribution::oHF(double x) {
    return std::min(Utility::mathRound(x / o_min_gap), (double)(od_complex_degree.size() - 1));
}

int_t Distribution::iHF(double x) {
    return std::min(Utility::mathRound(x / i_min_gap), (double)(id_hash_tid.size() - 1));
}

int_t Distribution::genOutDegreeSimple(int_t id) {
    if (od_simple_cnum.empty()) {
        buildForOdSimply();
    }
    if (id == od_simple_mem_no) {
        od_simple_cur_no = od_simple_mem_no;
        od_simple_cur_id = od_simple_mem_id;
        od_simple_offset = od_simple_mem_off;
        return od_simple_cur_id + min_degree;
    } else if (id == od_simple_cur_no + 1) {
        od_simple_cur_no = id;
        if (od_simple_cur_id < od_simple_cnum.size()) {
            if (od_simple_offset < od_simple_cnum[(int)od_simple_cur_id]) {
                od_simple_offset ++;
            } else {
                od_simple_offset = 1;
                od_simple_cur_id ++;
            }
            return od_simple_cur_id + min_degree;
        } else {
            return od_simple_cur_id + min_degree - 1;
        }
    } else {
        int_t no = od_simple_mem_no + od_simple_cnum[od_simple_mem_id] - od_simple_mem_off;
        int index = od_simple_mem_id;
        while (index < od_simple_cnum.size() && no < id) {
            no += od_simple_cnum[++ index];
        }
        return index + min_degree - 1;
    }
}

int_t Distribution::genOutDegreeComplex(int_t id) {
    double rv = (id / bucket) * bucket_step + rand.nextReal() * bucket_step;
    int index = (int)std::min(oHF(rv), (int_t)od_complex_degree.size() - 1);
    return od_complex_degree[index];
}

int_t Distribution::genOutDegree(int_t id) {
    return genOutDegreeComplex(id);
}

int_t Distribution::genTargetID() {
    double rv = std::max(rand.nextReal(), i_min_rv);
    int index = (int)iHF(rv);
    int_t a;
    double c, r;
    if (rv >= id_hash_cdf[index]) {
        a = id_hash_tid[index];
        c = id_hash_cdf[index];
        r = id_hash_ratio[index];
    } else {
        a = id_hash_tid[index - 1];
        c = id_hash_cdf[index - 1];
        r = id_hash_ratio[index - 1];
    }
    return a + Utility::mathRound((rv - c) * r);
}

// =================== For Debug ====================

double Distribution::getOMinGap() {
    return o_min_gap;
}

double Distribution::getIMinGap() {
    return i_min_gap;
}

void Distribution::printICdf() {
    for (auto n : id_hash_cdf)
        std::cout << n << " ";
    std::cout << std::endl;
}

void Distribution::printOCdf() {
    int size = od_simple_cnum.size();
    std::vector<double> cdf(size);
    cdf[0] = od_simple_cnum[0];
    for (int i = 1; i < size; ++i)
        cdf[i] = cdf[i - 1] + od_simple_cnum[i];
    for (int i = 0; i < size; ++i)
        cdf[i] /= cdf[size - 1];
    double ans = 1.0;
    for (int i = 0; i < size; ++i) {
        std::cout << cdf[i] << " ";
        if (i > 0)
            ans = std::min(ans, cdf[i] - cdf[i - 1]);
    }
    std::cout << "[Distribution::printOCdf] Min out gap = " << ans << std::endl;
}

// for multiple threads, deprecated
std::vector<int> Distribution::splitSourceNodes(int n_threads) {
    std::vector<int> ans(n_threads + 1);
    ans[0] = 0;
    int_t step = (int_t)(num_edges / n_threads);
    int_t vc = step;
    int j = 0;
    int d = min_degree;
    int_t sum = 0;
    int acc = 0;
    for (int i = 1; i < n_threads; ++i) {
        while (j < od_simple_cnum.size() && sum < vc) {
            sum += d * od_simple_cnum[j];
            acc += (int)od_simple_cnum[j];
            d ++;
            j ++;
        }
        vc += step;
        ans[i] = acc;
    }
    ans[n_threads] = (int)num_nodes;
    return ans;
}

std::vector<int_t> Distribution::getOdNum() {
    return od_simple_cnum;
}

} //! namespace fastsgg
} //! namespace gl