/**
 * create time : 2020-12-19
 * Attribute.hpp : generate attributes
 */
#include "Attribute.hpp"

namespace gl {
namespace fastsgg {

// ============== st_Value ==================
st_Value::st_Value(int size) {
    int len = 1 + (size / 32);
    _mt_value.resize(len);
}

void st_Value::set_bit(int pos) {   // START FROM 0
    int i = (pos / 32);
    int j = (pos % 32);
    if (i < _mt_value.size()) {
        _mt_value[i] |= (1 << j);
    }
}

bool st_Value::has_bit(int pos) {   // START FROM 0
    int i = (pos / 32);
    int j = (pos % 32);
    if (i < _mt_value.size()) {
        return (_mt_value[i] & (1 << j));
    }
    return false;
}

bool st_Value::empty() const {
    for (auto n : _mt_value) {
        if (n != 0) {
            return false;
        }
    }
    return true;
}

size_t st_Value::size() const {
    return _mt_value.size();
}

uint32_t st_Value::operator[](int i) const {
    if (i < _mt_value.size()) {
        return _mt_value[i];
    }
    return 0;
}

std::string st_Value::get_string(char sep) const {
    std::string ans = "";
    size_t len = _mt_value.size();
    int acc = 0, num = 0;
    for (size_t i = 0; i < len; ++i) {
        int xn = _mt_value[i];
        for (int j = 0; (xn != 0) && (j < 32); ++j) {
            if (xn & (1 << j)) {
                xn = xn & (~(1 << j));
                num = acc + j;
                if (ans.empty()) {
                    ans = std::to_string(num);
                } else {
                    ans = ans + sep + std::to_string(num);
                }
            }
        }
        acc += 32;
    }
    return ans;
}

// ============== st_Prop ==================
st_Prop::st_Prop(int n_many_values, const std::vector<int>& n_multi_values) {
    // many value
    vec_many_values.resize(n_many_values, -1);
    // multi value
    size_t len = n_multi_values.size();
    vec_multi_values.resize(len);
    for (size_t i = 0; i < len; ++i) {
        vec_multi_values[i] = new st_Value(n_multi_values[i]);
    }
}

void st_Prop::set_prop(int idx, int val, bool b_many) {   // val: START FROM 0
    if (b_many) {
        if (0 <= idx && idx < vec_many_values.size()) {
            vec_many_values[idx] = val;
        }
    } else {
        if (0 <= idx && idx < vec_multi_values.size()) {
            vec_multi_values[idx]->set_bit(val);
        }
    }
}

bool st_Prop::has_prop(int idx, int val, bool b_many) {   // val: START FROM 0
    if (b_many) {
        if (0 <= idx && idx < vec_many_values.size()) {
            return (vec_many_values[idx] == val);
        }
    } else {
        if (0 <= idx && idx < vec_multi_values.size()) {
            return vec_multi_values[idx]->has_bit(val);
        }
    }
    return false;
}

bool st_Prop::is_empty(int idx, bool b_many) {
    if (b_many) {
        if (0 <= idx && idx < vec_many_values.size()) {
            return (vec_many_values[idx] == -1);
        }
    } else {
        if (0 <= idx && idx < vec_multi_values.size()) {
            return vec_multi_values[idx]->empty();
        }
    }
    return false;
}

size_t st_Prop::size() const { // many_value, #keys
    return vec_many_values.size();
}

size_t st_Prop::length() const {   // multi_value, #keys
    return vec_multi_values.size();
}

int st_Prop::operator[](int idx) const {
    if (idx >= 0 && idx < vec_many_values.size()) {
        return vec_many_values[idx];
    }
    return 0;
}

st_Value st_Prop::operator()(int idx) const {
    if (idx >= 0 && idx < vec_multi_values.size()) {
        return (*(vec_multi_values[idx]));
    }
    return st_Value();
}

std::string st_Prop::get_string() const {
    std::string ans = "";
    char one_key_sep = ',';
    char amg_key_sep = '\t';
    // many values
    size_t len_many = vec_many_values.size();
    if (!vec_many_values.empty()) {
        ans = std::to_string(vec_many_values[0]);
    }
    for (size_t i = 1; i < len_many; ++i) {
        ans = ans + amg_key_sep + std::to_string(vec_many_values[i]);
    }
    // multiple values
    size_t len_multi = vec_multi_values.size();
    size_t i = 0;
    if (ans.empty() && len_multi > 0) {
        ans = vec_multi_values[0]->get_string(one_key_sep);
        i = 1;
    }
    for (; i < len_multi; ++i) {
        ans = ans + amg_key_sep + vec_multi_values[i]->get_string(one_key_sep);
    }
    return ans;
}
// ============== END st_Prop ==================

bool operator==(const st_Value& lhs, const st_Value& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    size_t len = lhs.size();
    for (size_t i = 0; i < len; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

int scoreOfst_Value(const st_Value& lhs, const st_Value& rhs) {
    if (lhs.size() != rhs.size()) {
        return 0;
    }
    size_t len = lhs.size();
    int ans = 0;
    for (size_t i = 0; i < len; ++i) {
        ans += Utility::numOneBitInt(lhs[i] & rhs[i]);
    }
    return ans;
}

int orScoreOfst_Value(const st_Value& lhs, const st_Value& rhs) {
    if (lhs.size() != rhs.size()) {
        return 0;
    }
    size_t len = lhs.size();
    int ans = 0;
    for (size_t i = 0; i < len; ++i) {
        ans += Utility::numOneBitInt(lhs[i] | rhs[i]);
    }
    return ans;
}

std::ostream& operator<<(std::ostream& os, const st_Prop& prop) {
    os << prop.get_string() << std::endl;
    return os;
}

int scoreOfProp(const st_Prop& lhs, const st_Prop& rhs) {
    size_t len_many = lhs.size();
    size_t len_multi = lhs.length();
    if (len_many != rhs.size() || len_multi != rhs.length()) {
        return 0;
    }
    int ans = 0;
    for (size_t i = 0; i < len_many; ++i) {
        if (lhs[i] == rhs[i]) {
            ans ++;
        }
    }
    for (size_t i = 0; i < len_multi; ++i) {
        ans += scoreOfst_Value(lhs(i), rhs(i));
    }
    return ans;
}

int orScoreOfProp(const st_Prop& lhs, const st_Prop& rhs) { // just for multi values
    int len_multi = lhs.length();
    if (len_multi != rhs.length()) {
        return 0;
    }
    int ans = 0;
    for (size_t i = 0; i < len_multi; ++i) {
        ans += orScoreOfst_Value(lhs(i), rhs(i));
    }
    return ans;
}

Attribute::Attribute() {

}

Attribute::~Attribute() {

}

Attribute::Attribute(std::vector<int>& many_matrix, std::vector<int>& multi_matrix) {
    many_attribute = many_matrix;
    multi_attribute = multi_matrix;
    b_many_condition.resize(many_matrix.size(), false);
    
    n_select_attr_upper = n_center_attr_upper = 0;
    n_select_attr_upper += many_matrix.size();
    for (auto n : multi_matrix) {
        n_select_attr_upper += n;
    }
    n_center_attr_upper = n_select_attr_upper;
}

// just for many values
// P( condition_j | base_i )
bool Attribute::add_conditional_dist(
    int base_i, int condition_j, std::vector<std::vector<double>>& probability) {
    if (base_i >= many_attribute.size() ||
        condition_j >= many_attribute.size()) {
        return false;
    }

    // normalization and build CDF
    int n_row = probability.size();
    int n_col = 0;
    if (n_row > 0) {
        n_col= probability[0].size();
    }
    if (n_row != many_attribute[base_i] || n_col != many_attribute[condition_j]) {
        std::cerr << "[Attribute::add_conditional_dist] Probability Matrix size Error, should be " << n_row << " X " << n_col << std::endl;
        return false;
    }
    for (int i = 0; i < n_row; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n_col; ++j) {
            probability[i][j] = fabs(probability[i][j]);
            sum += probability[i][j];
        }
        if (sum == 0.0) {
            double d_unit = 1.0 / (double)(n_col);
            if (n_col > 0) {
                probability[i][0] = d_unit;
            }
            for (int j = 1; j < n_col; ++j) {
                probability[i][j] = probability[i][j - 1] + d_unit;
            }
        } else {
            if (n_col > 0) {
                probability[i][0] /= sum;
            }
            for (int j = 1; j < n_col; ++j) {
                probability[i][j] = probability[i][j - 1] + probability[i][j] / sum;
            }
        }
    }
    probability_map[condition_j][base_i] = probability;

    b_many_condition[condition_j] = true;
    n_center_attr_upper --;
    return true;
}

void Attribute::build_comm_centers(int n_comms, int K) {
    K = std::min(K, n_center_attr_upper);
    std::vector<int> cand_list(n_center_attr_upper);
    std::vector<int> cand_type(n_center_attr_upper, -1);
    int len_many = many_attribute.size();
    int ci = 0;
    for (int i = 0; i < len_many; ++i) {
        if (b_many_condition[i]) {
            continue;
        }
        cand_list[ci ++] = i;
    }
    int len_multi = multi_attribute.size();
    for (int i = 0; i < len_multi; ++i) {
        for (int j = 0; j < multi_attribute[i]; ++j) {
            cand_list[ci] = j;
            cand_type[ci] = i;
            ci ++;
        }
    }
    comm_center_prop.resize(n_comms);
    for (int i = 0; i < n_comms; ++i) {
        comm_center_prop[i] = new st_Prop(len_many, multi_attribute);
        Utility::randomChoice(cand_list, cand_type, K);
        for (int j = 0; j < K; ++j) {
            if (cand_type[j] == -1) {   // many values
                comm_center_prop[i]->set_prop( cand_list[j],
                    rand.nextInt(many_attribute[cand_list[j]] - 1) );
            } else {    // multi values
                comm_center_prop[i]->set_prop(cand_type[j], cand_list[j], false);
            }
        }
    }
}

void Attribute::fill_st_prop(st_Prop& ans) {
    int len_many = many_attribute.size();
    int len_multi = multi_attribute.size();
    // many values generation
    for (int i = 0; i < len_many; ++i) {
        if (!b_many_condition[i] && ans.is_empty(i)) {
            ans.set_prop(i, rand.nextInt(many_attribute[i] - 1));
        }
    }
    for (int i = 0; i < len_many; ++i) {
        if (b_many_condition[i]) {  // conditional distribution
            if (probability_map.count(i)) {
                int bi = probability_map[i].begin()->first;
                int bi_val = ans[bi];
                d_matrix& prob_mat = probability_map[i][bi];
                double prop = rand.nextReal();
                auto lit = std::lower_bound(prob_mat[bi_val].begin(), prob_mat[bi_val].end(), prop);
                int val = lit - prob_mat[bi_val].begin();
                ans.set_prop(i, val);
            } else {
                ans.set_prop(i, rand.nextInt(many_attribute[i] - 1));
            }
        }
    }
    // multi values generation, 如果有属性了，就不生成了，为后续调整留空间; 否则，随机生成一个
    for (int i = 0; i < len_multi; ++i) {
        if (ans.is_empty(i, false)) {
            ans.set_prop(i, rand.nextInt(multi_attribute[i] - 1), false);
        }
    }
}

st_Prop Attribute::gen_one_comm_prop(int comm_id) {
    st_Prop ans;
    if (!(comm_id >=0 && comm_id < comm_center_prop.size()))
        return ans;
        // return st_Prop();
        // return st_Prop(0, std::vector<int>());
    ans = (*(comm_center_prop[comm_id]));
    fill_st_prop(ans);
    return ans;
}

st_Prop Attribute::gen_mul_comm_prop(std::vector<int>& comm_id_list) {
    st_Prop ans;
    if (comm_id_list.empty())
        return ans;
    int len_many = many_attribute.size();
    int len_multi = multi_attribute.size();
    // 构建公共的 st_Prop
    bool first = true;
    for (auto ci : comm_id_list) {
        if (ci >= 0 && ci < comm_center_prop.size()) {
            if (first) {
                ans = (*(comm_center_prop[ci]));
                first = false;
            } else {
                // many values
                for (int i = 0; i < len_many; ++i) {
                    if (ans.is_empty(i) && !comm_center_prop[ci]->is_empty(i)) {
                        ans.set_prop(i, (*(comm_center_prop[ci]))[i]);
                    }
                }
                // multi values
                for (int i = 0; i < len_multi; ++i) {
                    if (ans.is_empty(i, false) && !comm_center_prop[ci]->is_empty(i, false)) {
                        for (int j = 0; j < multi_attribute[i]; ++j) {
                            if ((*(comm_center_prop[ci]))(i).has_bit(j)) {
                                ans.set_prop(i, j, false);
                            }
                        }
                    }
                } // end multi
            } // end merge
        }
    }
    fill_st_prop(ans);
    return ans;
}

void Attribute::adjust(st_Prop& one_prop, st_Prop& ano_prop, double sim) {
    int len_many = many_attribute.size();
    int len_multi = multi_attribute.size();
    int score = scoreOfProp(one_prop, ano_prop);
    int multi_or = orScoreOfProp(one_prop, ano_prop);
    int denom = multi_or + len_many;
    double cur_sim = (double)(score) / (double)(denom);
    if (cur_sim >= sim) {
        return;
    }
    int upper = n_select_attr_upper - len_many - multi_or;
    double d_max_sim = (double)(score + upper) / (double)(n_select_attr_upper);
    if (d_max_sim < sim) { // all
        return;
    }
    // naive method
    int K = 0;
    for (K = 1; K <= upper; ++K) {
        double u_sim = (double)(score + K) / (double)(denom + K);
        if (u_sim >= sim) {
            break;
        }
    }
    // random K
    std::vector<int> cand_list;
    std::vector<int> cand_type;
    for (int i = 0; i < len_multi; ++i) {
        for (int j = 0; j < multi_attribute[i]; ++j) {
            if (!one_prop(i).has_bit(j) && !ano_prop(i).has_bit(j)) {
                cand_list.push_back(j);
                cand_type.push_back(i);
            }
        }
    }
    Utility::randomChoice(cand_list, cand_type, K);
    for (int j = 0; j < K; ++j) {
        one_prop.set_prop(cand_type[j], cand_list[j], false);
        ano_prop.set_prop(cand_type[j], cand_list[j], false);
    }
}

} //! namespace fastsgg
} //! namespace gl