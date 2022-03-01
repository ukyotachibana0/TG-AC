/**
 * create time : 2020-12-17
 * Attribute.hpp : Node/Edge Attributes
 */
#pragma once

#include "headers.hpp"

namespace gl {
namespace fastsgg {

class st_Value {
private:
    std::vector<uint32_t> _mt_value;

public:
    st_Value() {}
    ~st_Value() {}

    st_Value(int size);

    void set_bit(int pos); // START FROM 0

    bool has_bit(int pos); // START FROM 0

    bool empty() const;

    size_t size() const;

    uint32_t operator[](int i) const;

    std::string get_string(char sep=',') const;
};

bool operator==(const st_Value& lhs, const st_Value& rhs);

int scoreOfst_Value(const st_Value& lhs, const st_Value& rhs);

int orScoreOfst_Value(const st_Value& lhs, const st_Value& rhs);

class st_Prop {
private:
    std::vector<int> vec_many_values;
    std::vector<st_Value*> vec_multi_values;

public:
    st_Prop() {}
    ~st_Prop() {}

    st_Prop(int n_many_values, const std::vector<int>& n_multi_values);

    void set_prop(int idx, int val, bool b_many=true);  // val: START FROM 0

    bool has_prop(int idx, int val, bool b_many=true);  // val: START FROM 0

    bool is_empty(int idx, bool b_many=true);

    size_t size() const;    // many_value, #keys

    size_t length() const;  // multi_value, #keys

    int operator[](int idx) const;

    st_Value operator()(int idx) const;

    std::string get_string() const;
};

std::ostream& operator<<(std::ostream& os, const st_Prop& prop);

int scoreOfProp(const st_Prop& lhs, const st_Prop& rhs);

// just for multi values
int orScoreOfProp(const st_Prop& lhs, const st_Prop& rhs);

class Attribute
{
public:
    Attribute();
    ~Attribute();

    Attribute(std::vector<int>& many_matrix, std::vector<int>& multi_matrix);

    // just for many values
    // P( condition_j | base_i )
    bool add_conditional_dist(int base_i, int conditon_j, std::vector<std::vector<double>>& probability);

    // 构建 n_comms 个虚拟的社区中心节点属性
    // K: 相同属性的数量
    void build_comm_centers(int n_comms, int K);

    // 随机生成一个社区 comm_id 中的节点属性
    st_Prop gen_one_comm_prop(int comm_id);

    // 随机生成在重叠社区 comm_id_list 中的节点属性
    st_Prop gen_mul_comm_prop(std::vector<int>& comm_id_list);

    // 给定不在同一社区中的有边相连的两个节点属性，调整，即，保证两个节点的一定相似性 sim
    void adjust(st_Prop& one_prop, st_Prop& ano_prop, double sim);

private:
    void fill_st_prop(st_Prop& ans);

private:
    Random rand;

    std::vector<st_Prop*> comm_center_prop;  // 社区虚拟中心节点
    std::vector<int> many_attribute;        // 每个many属性的可选值数量
    std::vector<bool> b_many_condition;     // 是否为涉及到条件分布中的属性
    std::vector<int> multi_attribute;       // 每个multi属性的可选值数量

    int n_select_attr_upper;    // 一个节点/边 能够具有的属性数量 上限
    int n_center_attr_upper;    // 虚拟中心节点 具有的节点数量上限，去除 涉及到 条件分布的属性

    using d_matrix = std::vector<std::vector<double>>;
    // condition -> base, P ( condition | base )
    std::unordered_map<int, std::unordered_map<int, d_matrix>> probability_map;

}; //! class Attribute

} //! namespace fastsgg
} //! namespace gl
