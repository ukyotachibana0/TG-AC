/**
 * create time : 2020-01-12
 * gen_attribute.cpp : generate attributes
 */
#include "gen_attribute.hpp"
#include "Attribute.hpp"

namespace gl {
namespace fastsgg {

void show_st_Cond(const st_Cond& cond_st) {
    std::cout << "P( " << cond_st.cond_attr << " | " << cond_st.base_attr  << " ): ";
    for (auto& row : cond_st.prop) {
        for (auto n : row) {
            std::cout << n << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void show_st_GenAttr(const st_GenAttr& attr_st) {
    std::cout << "#Nodes = " << attr_st.num_nodes << std::endl;
    std::cout << "#Communities = " << attr_st.k_community << std::endl;
    std::cout << "Community: ";
    for (auto n : attr_st.comm_split) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    std::cout << "Many value: ";
    for (auto& elem : attr_st.many_values) {
        std::cout << elem.first << " -> " << elem.second << std::endl;
    }
    std::cout << "Multi value: ";
    for (auto& elem : attr_st.multi_values) {
        std::cout << elem.first << " -> " << elem.second << std::endl;
    }
    for (auto& st : attr_st.constraints) {
        show_st_Cond(st);
    }
}

// 根据 st_GenAttr 中的属性信息生成属性，输出到 output_fn 文件中，edge_fn 为图的拓扑结构
// 根据拓扑结构确定社区间的边
bool gen_attr(st_GenAttr& attr_st, std::string& output_fn, std::string& edge_fn) {
    std::vector<std::string> many_label;
    std::vector<int> many_opts;
    std::unordered_map<std::string, int> many_id_map;
    std::vector<std::string> multi_label;
    std::vector<int> multi_opts;
    std::unordered_map<std::string, int> multi_id_map;
    for (auto& elem : attr_st.many_values) {
        many_id_map[elem.first] = many_opts.size();
        many_label.push_back(elem.first);
        many_opts.push_back(elem.second);
    }
    for (auto& elem : attr_st.multi_values) {
        multi_id_map[elem.first] = multi_opts.size();
        multi_label.push_back(elem.first);
        multi_opts.push_back(elem.second);
    }
    // prepare
    Attribute attr_ins(many_opts, multi_opts);
    for (auto& one_cond : attr_st.constraints) {
        attr_ins.add_conditional_dist(many_id_map[one_cond.base_attr],
            many_id_map[one_cond.cond_attr], one_cond.prop);
    }
    int K_sim = many_opts.size() / 2;
    attr_ins.build_comm_centers(attr_st.k_community, K_sim);
    std::cout << "Prepare done." << std::endl;
    // generate
    size_t len = attr_st.comm_split.size();
    std::vector<st_Prop> node_attr_list(attr_st.num_nodes);
    for (size_t i = 0; i < len - 1; ++i) {
        int_t lid = attr_st.comm_split[i];
        int_t rid = attr_st.comm_split[i + 1];
        for (int_t nid = lid; nid < rid; ++nid) {
            node_attr_list[nid] = attr_ins.gen_one_comm_prop(i);
        }
    }
    std::cout << "Generate done." << std::endl;
    // adjust
    double sim = 0.6;
    std::ifstream fin(edge_fn.c_str());
    if (!fin.is_open()) {
        std::cerr << "Cannot open " << edge_fn << std::endl;
        return false;
    }
    auto comm_id = [](int_t val, const std::vector<int_t>& vec) {
        auto it = std::lower_bound(vec.begin(), vec.end(), val);
        return (it - vec.begin());
    };
    int_t src_id, tgt_id;
    while (fin >> src_id >> tgt_id) {
        int s_cid = comm_id(src_id, attr_st.comm_split);
        int t_cid = comm_id(tgt_id, attr_st.comm_split);
        if (s_cid != t_cid) {
            attr_ins.adjust(node_attr_list[s_cid], node_attr_list[t_cid], sim);
        }
    }
    std::cout << "Adjust done." << std::endl;
    // flush
    std::ofstream fout(output_fn.c_str());
    fout << "Many_Value: ";
    if (many_label.size()) {
        fout << many_label[0];
    }
    for (size_t i = 1; i < many_label.size(); ++i) {
        fout << "," << many_label[i];
    }
    fout << ";Multi_Value: ";
    if (multi_label.size()) {
        fout << multi_label[0];
    }
    for (size_t i = 1; i < multi_label.size(); ++i) {
        fout << "," << multi_label[i];
    }
    fout << std::endl;
    for (int_t i = 0; i < attr_st.num_nodes; ++i) {
        fout << node_attr_list[i];
    }
    fout.close();
    std::cout << "Flush done." << std::endl;
    return true;
}

// 根据输入的属性 JSON 文件构建 st_GenAttr 结构体 attr_st
bool build_st_GenAttr(std::string attr_json, std::string comm_fn, st_GenAttr& attr_st) {
    std::ifstream fin(attr_json.c_str());
    if (!fin.is_open()) {
        std::cerr << "Cannot open attribute JSON file: " << attr_json << std::endl;
        return false;
    }
    JSON::json json_obj;
    fin >> json_obj;
    // many values
    if (json_obj.find(AttrSchema::json_many_value) != json_obj.end()) {
        if (!json_obj[AttrSchema::json_many_value].is_array()) {
            std::cerr << "`many_value` type should be list" << std::endl;
            return false;
        }
        auto& many_value_list = json_obj[AttrSchema::json_many_value];
        for (auto& one_many : many_value_list) {
            if (one_many.find(AttrSchema::json_attr_label) == one_many.end()) {
                std::cerr << "Many value lack of " << AttrSchema::json_attr_label << std::endl;
                return false;
            }
            if (!one_many[AttrSchema::json_attr_label].is_string()) {
                std::cerr << "Many value label type: string" << std::endl;
                return false;
            }
            if (one_many.find(AttrSchema::json_attr_opts) == one_many.end()) {
                std::cerr << "Many value lack of " << AttrSchema::json_attr_opts << std::endl;
                return false;
            }
            if (!one_many[AttrSchema::json_attr_opts].is_number()) {
                std::cerr << "Many value num_opt type: number" << std::endl;
                return false;
            }
            attr_st.many_values[one_many[AttrSchema::json_attr_label]] = one_many[AttrSchema::json_attr_opts];
        }
    }
    // multi values
    if (json_obj.find(AttrSchema::json_multi_value) != json_obj.end()) {
        if (!json_obj[AttrSchema::json_multi_value].is_array()) {
            std::cerr << "`multi_value` type should be list" << std::endl;
            return false;
        }
        auto& multi_value_list = json_obj[AttrSchema::json_multi_value];
        for (auto& one_multi : multi_value_list) {
            if (one_multi.find(AttrSchema::json_attr_label) == one_multi.end()) {
                std::cerr << "Multi value lack of " << AttrSchema::json_attr_label << std::endl;
                return false;
            }
            if (!one_multi[AttrSchema::json_attr_label].is_string()) {
                std::cerr << "Multi value label type: string" << std::endl;
                return false;
            }
            if (one_multi.find(AttrSchema::json_attr_opts) == one_multi.end()) {
                std::cerr << "Multi value lack of " << AttrSchema::json_attr_opts << std::endl;
                return false;
            }
            if (!one_multi[AttrSchema::json_attr_opts].is_number()) {
                std::cerr << "Multi value num_opt type: number" << std::endl;
                return false;
            }
            attr_st.multi_values[one_multi[AttrSchema::json_attr_label]] = one_multi[AttrSchema::json_attr_opts];
        }
    }
    // constraints
    if (json_obj.find(AttrSchema::json_attr_cons) != json_obj.end()) {
        if (!json_obj[AttrSchema::json_attr_cons].is_array()) {
            std::cerr << "`many_constraint` type should be list" << std::endl;
            return false;
        }
        auto& many_constraint_list = json_obj[AttrSchema::json_attr_cons];
        for (auto& one_cons : many_constraint_list) {
            if (one_cons.find(AttrSchema::json_cons_base) == one_cons.end() ||
                !one_cons[AttrSchema::json_cons_base].is_string()) {
                std::cerr << "Many constraint `base` label type: string" << std::endl;
                return false;
            }
            if (one_cons.find(AttrSchema::json_cons_cond) == one_cons.end() ||
                !one_cons[AttrSchema::json_cons_cond].is_string()) {
                std::cerr << "Many constraint `condition` label type: string" << std::endl;
                return false;
            }
            if (one_cons.find(AttrSchema::json_cons_prob) == one_cons.end() ||
                !one_cons[AttrSchema::json_cons_prob].is_array()) {
                std::cerr << "Many constraint `prob` type: 2-d array" << std::endl;
                return false;
            }
            st_Cond a_st_cond;
            a_st_cond.base_attr = one_cons[AttrSchema::json_cons_base];
            a_st_cond.cond_attr = one_cons[AttrSchema::json_cons_cond];
            std::vector<std::vector<double>> axt = one_cons[AttrSchema::json_cons_prob];
            // a_st_cond.prop = one_cons[AttrSchema::json_cons_prob];
            a_st_cond.prop = axt;
            attr_st.constraints.push_back(a_st_cond);
        }
    }
    // community split
    fin.close();
    fin.open(comm_fn.c_str());
    if (!fin.is_open()) {
        std::cerr << "Cannot open community split file: " << comm_fn << std::endl;
        return false;
    }
    std::string line;
    std::istringstream iss("");
    int_t row_id, col_id;
    while (std::getline(fin, line)) {
        iss.clear();
        iss.str(line);
        iss >> row_id >> col_id;
        if (row_id != col_id) {
            std::cerr << "Row id " << row_id << " != " << "Col id " << col_id << std::endl;
        }
        attr_st.comm_split.push_back(row_id);
    }
    size_t len = attr_st.comm_split.size();
    attr_st.k_community = len - 1;
    attr_st.num_nodes = attr_st.comm_split[len - 1];
    return true;
}

// 生成的主程序:
// 输入：属性 JSON 文件 attr_json，社区划分的文件 comm_fn，输出属性的文件 output_fn，网络边集文件 edge_fn
// 输出：是否生成成功
bool gen_main(std::string attr_json, std::string comm_fn, std::string output_fn, std::string edge_fn) {
    st_GenAttr attr_st;
    bool res = build_st_GenAttr(attr_json, comm_fn, attr_st);
    if (!res) {
        return res;
    }
    // show_st_GenAttr(attr_st);
    res = gen_attr(attr_st, output_fn, edge_fn);
    return res;
}

} //! namespace fastsgg
} //! namespace gl