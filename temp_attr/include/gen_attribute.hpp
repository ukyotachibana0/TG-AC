/**
 * create time : 2020-01-13
 * gen_attribute.hpp : temporary
 */
#pragma once
#include "Attribute.hpp"
#include "types.hpp"

namespace gl {
namespace fastsgg {

namespace AttrSchema {
    const static std::string json_many_value = "many_value";
    const static std::string json_attr_label = "label";
    const static std::string json_attr_opts = "num_opt";
    const static std::string json_attr_cons = "many_constraint";
    const static std::string json_cons_base = "base";
    const static std::string json_cons_cond = "condition";
    const static std::string json_cons_prob = "prob";
    const static std::string json_multi_value = "multi_value";
}

// 涉及到条件分布的属性结构体
typedef struct _condition_st {
    std::string base_attr;
    std::string cond_attr;
    std::vector<std::vector<double>> prop;
} st_Cond;

// 用于生成属性的信息构成的结构体
typedef struct _gen_attr_st {
    int_t num_nodes;
    int k_community;
    std::vector<int_t> comm_split;
    std::unordered_map<std::string, int> many_values;
    std::unordered_map<std::string, int> multi_values;
    std::vector<st_Cond> constraints;
} st_GenAttr;

void show_st_Cond(const st_Cond& cond_st);

void show_st_GenAttr(const st_GenAttr& attr_st);

bool gen_attr(st_GenAttr& attr_st, std::string& output_fn);

bool build_st_GenAttr(std::string attr_json, std::string comm_fn, st_GenAttr& attr_st);

bool gen_main(std::string attr_json, std::string comm_fn, std::string output_fn, std::string edge_fn);

} //! namespace fastsgg
} //! namespace gl