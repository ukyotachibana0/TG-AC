/**
 * create time : 2020-10-22
 * Generation implementation
 */
#include "Generation.hpp"
#include "PowerLaw.hpp"
#include "Normal.hpp"
#include "LogNormal.hpp"
#include "Uniform.hpp"
#include "Store.hpp"
#include "OutDegreeDistribution.hpp"
#include "DeltaOutDistribution.hpp"
#include "OutLogNormal.hpp"
#include "OutPowerLaw.hpp"
#include "OutNormal.hpp"
#include "headers.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <omp.h>
#include <list>

namespace gl {
namespace fastsgg {

Generation::Generation() {
    // pass
    gp_progress = 0.0;
    gp_tag = "";
    gb_gen_done = false;
    gb_start_gen = false;
}

Generation::Generation(std::string& filename) {
    json_filename = filename;
    std::ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        std::cerr << "[Generation::Generation(string)] Cannot open " << filename << std::endl;
        return;
    }
    fin >> json_obj;
    // #threads
    #pragma omp parallel
    {
        n_threads = omp_get_num_threads();
    }

#ifdef DEBUG
    std::cout << "#Threads = " << n_threads << std::endl;
#endif

    gp_progress = 0.0;
    gp_tag = "";
    gb_gen_done = false;
    gb_start_gen = false;
}

Generation::~Generation() {
    // pass
}

bool Generation::check_json() {
    // "graph": string
    bool ans = true;
    if (json_obj.find(schema::json_graph) == json_obj.end()) {
        std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_graph << "']." << std::endl;
        ans = false;
    }
    if (ans && !json_obj[schema::json_graph].is_string()) {
        std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_graph << "']." << std::endl;
        ans = false;
    }
    // node schema
    // "node": [{"label": string, "amount": number}, ...]
    if (json_obj.find(schema::json_node) == json_obj.end()) {
        std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_node << "']." << std::endl;
        return false;
    }
    if (!json_obj[schema::json_node].is_array()) {
        std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_node << "'] (List of objects)." << std::endl;
        return false;
    }
    auto& node_schema = json_obj[schema::json_node];
    std::unordered_set<std::string> node_set;
    for (auto& one_node : node_schema) {
        if (one_node.find(schema::json_node_label) == one_node.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_node << "'][i]['" << schema::json_node_label << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_node[schema::json_node_label].is_string()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_node << "'][i]['" << schema::json_node_label << "'] (string)." << std::endl;
            ans = false;
            continue;
        }
        if (node_set.count(one_node[schema::json_node_label])) {
            std::cerr << "[Generation::check_json] Content Error: JSON['" << schema::json_node << "'][i]['" << schema::json_node_label << "'] duplicated: " << one_node[schema::json_node_label] << std::endl;
            ans = false;
            continue;
        }
        std::string __node_label = one_node[schema::json_node_label];
        node_set.insert(__node_label);
        if (one_node.find(schema::json_node_amount) == one_node.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_node << "'][i]['" << schema::json_node_amount << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_node[schema::json_node_amount].is_number()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_node << "'][i]['" << schema::json_node_amount << "'] (number)." << std::endl;
            ans = false;
            continue;
        }
    }
    // edge schema
    // "edge": [{"label": string, "source": string, "target": string, "amount": number,
    //           "out": {}, "in"(optional): {}, "community"(optional): {}, 
    //           "temporal"(optional): {}, "embedded"(optional): {}}, ...]
    if (json_obj.find(schema::json_edge) == json_obj.end()) {
        std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "']." << std::endl;
        return false;
    }
    if (!json_obj[schema::json_edge].is_array()) {
        std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'] (List of objects)." << std::endl;
        return false;
    }
    auto& edge_schema = json_obj[schema::json_edge];
    std::unordered_set<std::string> edge_set;
    for (auto& one_edge : edge_schema) {
        // label
        if (one_edge.find(schema::json_edge_label) == one_edge.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_label << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_edge[schema::json_edge_label].is_string()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_label << "'] (string)." << std::endl;
            ans = false;
            continue;
        }
        if (edge_set.count(one_edge[schema::json_edge_label])) {
            std::cerr << "[Generation::check_json] Content Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_label << "'] duplicated: " << one_edge[schema::json_edge_label] << std::endl;
            ans = false;
            continue;
        }
        std::string __edge_label = one_edge[schema::json_edge_label];
        edge_set.insert(__edge_label);
        // source
        if (one_edge.find(schema::json_edge_source) == one_edge.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_source << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_edge[schema::json_edge_source].is_string()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_source << "'] (string)." << std::endl;
            ans = false;
            continue;
        }
        if (!node_set.count(one_edge[schema::json_edge_source])) {
            std::cerr << "[Generation::check_json] Content Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_source << "']." << std::endl;
            ans = false;
            continue;
        }
        // target
        if (one_edge.find(schema::json_edge_target) == one_edge.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_target << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_edge[schema::json_edge_target].is_string()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_target << "'] (string)." << std::endl;
            ans = false;
            continue;
        }
        if (!node_set.count(one_edge[schema::json_edge_target])) {
            std::cerr << "[Generation::check_json] Content Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_target << "']." << std::endl;
            ans = false;
            continue;
        }
        // amount
        if (one_edge.find(schema::json_edge_amount) == one_edge.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_amount << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_edge[schema::json_edge_amount].is_number()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_edge_amount << "'] (number)." << std::endl;
            ans = false;
            continue;
        }
        // temporal (optional)
        if (one_edge.find(schema::json_temp) != one_edge.end()) {
            if (!one_edge[schema::json_temp].is_object()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Object)." << std::endl;
                ans = false;
                continue;
            }
            auto& temp = one_edge[schema::json_temp];
            if (temp.find(schema::json_temp_type) == temp.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Type)." << std::endl;
                ans = false;
                continue;
            }
            if (!temp[schema::json_temp_type].is_string()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Type: string)." << std::endl;
                ans = false;
                continue;
            }
            std::string temp_type = temp[schema::json_temp_type];
            if (temp.find(schema::json_temp_min_timestamp) == temp.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Min-ts)." << std::endl;
                ans = false;
                continue;
            }
            if (!temp[schema::json_temp_min_timestamp].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Min-ts: number)." << std::endl;
                ans = false;
                continue;
            }
            if (temp.find(schema::json_temp_max_timestamp) == temp.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Max-ts)." << std::endl;
                ans = false;
                continue;
            }
            if (!temp[schema::json_temp_max_timestamp].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Max-ts: number)." << std::endl;
                ans = false;
                continue;
            }
            if (temp_type == schema::json_temp_PowerLaw) {
                if (temp.find(schema::json_dist_PL_lambda) == temp.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (PowerLaw lambda)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!temp[schema::json_dist_PL_lambda].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (PowerLaw lambda: number)." << std::endl;
                    ans = false;
                }
            } else if (temp_type == schema::json_temp_Normal || temp_type == schema::json_temp_LogNormal) {
                // mu(optional)
                if (temp.find(schema::json_dist_Nor_mu) != temp.end() && !temp[schema::json_dist_Nor_mu].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] ([Log]Normal mu: number)." << std::endl;
                    ans = false;
                    continue;
                }
                // sigma(required)
                if (temp.find(schema::json_dist_Nor_sigma) == temp.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] ([Log]Normal sigma)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!temp[schema::json_dist_Nor_sigma].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] ([Log]Normal sigma: number)." << std::endl;
                    ans = false;
                }
            } else if (temp_type == schema::json_temp_Uniform) {
                //
            } else {
                std::cerr << "[Generation::check_json] Unknown Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_temp << "'] (Unknown Distribution or not implement: " << temp_type << ")." << std::endl;
                ans = false;
            }
        }
        // out distribution
        if (one_edge.find(schema::json_out_dist) == one_edge.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!one_edge[schema::json_out_dist].is_object()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (Object)." << std::endl;
            ans = false;
            continue;
        }
        // Distribution
        auto& dist = one_edge[schema::json_out_dist];
        if (dist.find(schema::json_dist_type) == dist.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!dist[schema::json_dist_type].is_string()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (string)." << std::endl;
            ans = false;
            continue;
        }
        std::string dist_type = dist[schema::json_dist_type];
        if (dist.find(schema::json_dist_min_degree) == dist.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!dist[schema::json_dist_min_degree].is_number()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (number)." << std::endl;
            ans = false;
            continue;
        }
        if (dist.find(schema::json_dist_max_degree) == dist.end()) {
            std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "']." << std::endl;
            ans = false;
            continue;
        }
        if (!dist[schema::json_dist_max_degree].is_number()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (number)." << std::endl;
            ans = false;
            continue;
        }
        if (dist_type == schema::json_dist_PowerLaw) {
            if (dist.find(schema::json_dist_PL_lambda) == dist.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (PowerLaw lambda)." << std::endl;
                ans = false;
                continue;
            }
            if (!dist[schema::json_dist_PL_lambda].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (PowerLaw lambda number)." << std::endl;
                ans = false;
            }
        } else if (dist_type == schema::json_dist_Normal ||
                   dist_type == schema::json_dist_LogNormal) {
            if (dist.find(schema::json_dist_Nor_mu) == dist.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] ([Log]Normal mu)." << std::endl;
                ans = false;
                continue;
            }
            if (!dist[schema::json_dist_Nor_mu].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] ([Log]Normal mu number)." << std::endl;
                ans = false;
                continue;
            }
            if (dist.find(schema::json_dist_Nor_sigma) == dist.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] ([Log]Normal sigma)." << std::endl;
                ans = false;
                continue;
            }
            if (!dist[schema::json_dist_Nor_sigma].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] ([Log]Normal sigma number)." << std::endl;
                ans = false;
            }
        } else if (dist_type == schema::json_dist_Uniform) {
            //
        } else {
            std::cerr << "[Generation::check_json] Unknown Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_out_dist << "'] (Unknown Distribution or not implement: " << dist_type << ")." << std::endl;
            ans = false;
        }
        // in distribution (optional)
        if (one_edge.find(schema::json_in_dist) != one_edge.end()) {
            if (!one_edge[schema::json_in_dist].is_object()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (In-distribution object)." << std::endl;
                ans = false;
                continue;
            }
            // Distribution
            auto& dist = one_edge[schema::json_in_dist];
            if (dist.find(schema::json_dist_type) == dist.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "']." << std::endl;
                ans = false;
                continue;
            }
            if (!dist[schema::json_dist_type].is_string()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (Distribution type: string)." << std::endl;
                ans = false;
                continue;
            }
            std::string dist_type = dist[schema::json_dist_type];
            if (dist.find(schema::json_dist_min_degree) == dist.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "']." << std::endl;
                ans = false;
                continue;
            }
            if (!dist[schema::json_dist_min_degree].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (number)." << std::endl;
                ans = false;
                continue;
            }
            if (dist.find(schema::json_dist_max_degree) == dist.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "']." << std::endl;
                ans = false;
                continue;
            }
            if (!dist[schema::json_dist_max_degree].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (number)." << std::endl;
                ans = false;
                continue;
            }
            if (dist_type == schema::json_dist_PowerLaw) {
                if (dist.find(schema::json_dist_PL_lambda) == dist.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (PowerLaw lambda)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!dist[schema::json_dist_PL_lambda].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (PowerLaw lambda number)." << std::endl;
                    ans = false;
                }
            } else if (dist_type == schema::json_dist_Normal ||
                       dist_type == schema::json_dist_LogNormal) {
                if (dist.find(schema::json_dist_Nor_mu) == dist.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] ([Log]Normal mu)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!dist[schema::json_dist_Nor_mu].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] ([Log]Normal mu number)." << std::endl;
                    ans = false;
                    continue;
                }
                if (dist.find(schema::json_dist_Nor_sigma) == dist.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] ([Log]Normal sigma)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!dist[schema::json_dist_Nor_sigma].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] ([Log]Normal sigma number)." << std::endl;
                    ans = false;
                }
            } else if (dist_type == schema::json_dist_Uniform) {
                //
            } else {
                std::cerr << "[Generation::check_json] Unknown Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_in_dist << "'] (Unknown Distribution or not implement: " << dist_type << ")." << std::endl;
                ans = false;
            }
        }
        // community (optional)
        if (one_edge.find(schema::json_comm) != one_edge.end()) {
            if (!one_edge[schema::json_comm].is_object()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "'']." << std::endl;
                ans = false;
                continue;
            }
            auto& comm = one_edge[schema::json_comm];
            // community amount
            if (comm.find(schema::json_comm_amount) == comm.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "''] (amount)." << std::endl;
                ans = false;
                continue;
            }
            if (!comm[schema::json_comm_amount].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "'] (amount number)." << std::endl;
                ans = false;
                continue;
            }
            // rho
            if (comm.find(schema::json_comm_rho) == comm.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "'] (rho)." << std::endl;
                ans = false;
                continue;
            }
            if (!comm[schema::json_comm_rho].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "''] (rho number)." << std::endl;
                ans = false;
                continue;
            }
            // lambda
            if (comm.find(schema::json_comm_lambda) == comm.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "'] (lambda)." << std::endl;
                ans = false;
                continue;
            }
            if (!comm[schema::json_comm_lambda].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "''] (lambda number)." << std::endl;
                ans = false;
                continue;
            }
            // overlap (optional)
            if (comm.find(schema::json_comm_overlap) != comm.end()) {
                if (!comm[schema::json_comm_overlap].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_comm << "'] (overlap number)." << std::endl;
                    ans = false;
                    continue;
                }
            }
        }
        // embedded (optional)
        if (one_edge.find(schema::json_embd) != one_edge.end()) {
            if (!one_edge[schema::json_embd].is_object()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (object)." << std::endl;
                ans = false;
                continue;
            }
            auto& embd = one_edge[schema::json_embd];
            // type
            if (embd.find(schema::json_embd_type) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (type)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_type].is_string()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (type: string)." << std::endl;
                ans = false;
                continue;
            }
            std::string embd_type = embd[schema::json_embd_type];
            if (embd_type != schema::json_embd_map_neutral || embd_type != schema::json_embd_map_positive) {
                std::cerr << "[Generation::check_json] Unknown Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (Unknown type of embedding: " << embd_type << ")." << std::endl;
                ans = false;
                continue;
            }
            // window size
            if (embd.find(schema::json_embd_window_size) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (window size)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_window_size].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (window size: number)." << std::endl;
                ans = false;
                continue;
            }
            // source amount
            if (embd.find(schema::json_embd_srce_amount) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n source)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_srce_amount].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n source: number)." << std::endl;
                ans = false;
                continue;
            }
            // target amount
            if (embd.find(schema::json_embd_trgt_amount) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n target)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_trgt_amount].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n target: number)." << std::endl;
                ans = false;
                continue;
            }
            // edge amount
            if (embd.find(schema::json_embd_edge_amount) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n edge)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_edge_amount].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n edge: number)." << std::endl;
                ans = false;
                continue;
            }
            // embedded community amount
            if (embd.find(schema::json_embd_comm_amount) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n community)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_comm_amount].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (n community: number)." << std::endl;
                ans = false;
                continue;
            }
            // embedded community rho
            if (embd.find(schema::json_embd_comm_rho) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (rho)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_comm_rho].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (rho: number)." << std::endl;
                ans = false;
                continue;
            }
            // embedded community lambda
            if (embd.find(schema::json_embd_comm_lambda) == embd.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (lambda)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd[schema::json_embd_comm_lambda].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "'] (lambda: number)." << std::endl;
                ans = false;
                continue;
            }
            // temporal
            if (!embd[schema::json_embd_temp].is_object()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (object)." << std::endl;
                ans = false;
                continue;
            }
            auto& embd_temp = embd[schema::json_embd_temp];
            if (embd_temp.find(schema::json_temp_type) == embd_temp.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (type)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd_temp[schema::json_temp_type].is_string()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (type: string)." << std::endl;
                ans = false;
                continue;
            }
            std::string ebmd_temp_type = embd_temp[schema::json_temp_type];
            if (embd_temp.find(schema::json_temp_min_timestamp) == embd_temp.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (min-ts)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd_temp[schema::json_temp_min_timestamp].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (min-ts: number)." << std::endl;
                ans = false;
                continue;
            }
            if (embd_temp.find(schema::json_temp_max_timestamp) == embd_temp.end()) {
                std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (max-ts)." << std::endl;
                ans = false;
                continue;
            }
            if (!embd_temp[schema::json_temp_max_timestamp].is_number()) {
                std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (max-ts: number)." << std::endl;
                ans = false;
                continue;
            }
            if (ebmd_temp_type == schema::json_temp_PowerLaw) {
                if (embd_temp.find(schema::json_dist_PL_lambda) == embd_temp.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (Powerlaw lambda)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!embd_temp[schema::json_dist_PL_lambda].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (Powerlaw lambda: number)." << std::endl;
                    ans = false;
                }
            } else if (ebmd_temp_type == schema::json_temp_Normal || ebmd_temp_type == schema::json_temp_LogNormal) {
                // mu (optional)
                if (embd_temp.find(schema::json_dist_Nor_mu) != embd_temp.end() && !embd_temp[schema::json_dist_Nor_mu].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] ([Log]Normal mu: number)." << std::endl;
                    ans = false;
                    continue;
                }
                // sigma (required)
                if (embd_temp.find(schema::json_dist_Nor_sigma) == embd_temp.end()) {
                    std::cerr << "[Generation::check_json] Lack Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] ([Log]Normal sigma)." << std::endl;
                    ans = false;
                    continue;
                }
                if (!embd_temp[schema::json_dist_Nor_sigma].is_number()) {
                    std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] ([Log]Normal sigma: number)." << std::endl;
                    ans = false;
                }
            } else if (ebmd_temp_type == schema::json_temp_Uniform) {
                //
            } else {
                std::cerr << "[Generation::check_json] Unknown Error: JSON['" << schema::json_edge << "'][i]['" << schema::json_embd << "']['" << schema::json_embd_temp << "'] (Unknown Distribution or not implement: " << ebmd_temp_type << ")." << std::endl;
                ans = false;
            }
        }
    }
    // "gr"(optional): double
    if (json_obj.find(schema::json_gr) != json_obj.end()) {
        if (!json_obj[schema::json_gr].is_number()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_gr << "'] (gr double)." << std::endl;
            ans = false;
        }
    }
    // "store-format": string
    if (json_obj.find(schema::json_store_format) != json_obj.end()) {
        if (!json_obj[schema::json_store_format].is_string()) {
            std::cerr << "[Generation::check_json] Type Error: JSON['" << schema::json_store_format << "'] (store format string)." << std::endl;
            ans = false;
        }
    }
    return ans;
}

void Generation::generate_plan() {
    // graph name
    std::string graph_name = json_obj[schema::json_graph];
    mkdir(graph_name);
    // node schema
    auto& node_schema = json_obj[schema::json_node];
    std::unordered_map<std::string, int_t> node_label_amount;
    for (auto& one_node : node_schema) {
        std::string n_label = one_node[schema::json_node_label];
        int_t n_amount = one_node[schema::json_node_amount];
        node_label_amount[n_label] = n_amount;
    }
    // streaming?
    b_streaming_style = false;
    g_gr = 0.0;
    if (json_obj.find(schema::json_gr) != json_obj.end()) {
        b_streaming_style = true;
        g_gr = json_obj[schema::json_gr];
    }
    // storage format
    g_format = schema::json_format_TSV;
    if (json_obj.find(schema::json_store_format) != json_obj.end()) {
        g_format = json_obj[schema::json_store_format];
    }
    if (g_format == schema::json_format_TSV) {
        g_enum_format = EnumStoreFormat::TSV;
    } else if (g_format == schema::json_format_ADJ) {
        g_enum_format = EnumStoreFormat::ADJ;
    } else if (g_format == schema::json_format_CSR) {
        g_enum_format = EnumStoreFormat::CSR;
    } else {
        g_format = schema::json_format_TSV;
        g_enum_format = EnumStoreFormat::TSV;
    }
    // edge schema
    auto& edge_schema = json_obj[schema::json_edge];
    for (auto& one_edge : edge_schema) {
        // construct an St_EdgeGeneration
        St_EdgeGeneration st_one_edge;

        // Basic Information
        std::string e_label = one_edge[schema::json_edge_label];
        std::string e_source = one_edge[schema::json_edge_source];
        std::string e_target = one_edge[schema::json_edge_target];
        int_t s_nodes = node_label_amount[e_source];
        int_t t_nodes = node_label_amount[e_target];

        std::string sub_dir = graph_name + "/" + e_label;
        mkdir(sub_dir);

        std::string basename = sub_dir + "/" + e_source + "_" + e_target;
        std::string dump_filename = basename + "." + g_format;
        int_t e_amount = one_edge[schema::json_edge_amount];
        // # Information
        st_one_edge.e_label = e_label;
        st_one_edge.e_source = e_source;
        st_one_edge.e_target = e_target;
        st_one_edge.n_edges = e_amount;
        st_one_edge.s_nodes = s_nodes;
        st_one_edge.t_nodes = t_nodes;
        st_one_edge.filename = dump_filename;
        st_one_edge.basename = basename;

        // Temporal
        st_one_edge.b_temporal = false;
        if (one_edge.find(schema::json_temp) != one_edge.end()) {
            st_one_edge.b_temporal = true;
            auto& one_temporal = one_edge[schema::json_temp];
            st_one_edge.temp_type = one_temporal[schema::json_temp_type];
            std::unordered_map<std::string, double>& temp_params = st_one_edge.temp_params;
            temp_params[schema::json_temp_min_timestamp] = one_temporal[schema::json_temp_min_timestamp];
            temp_params[schema::json_temp_max_timestamp] = one_temporal[schema::json_temp_max_timestamp];
            if (one_temporal.find(schema::json_temp_PL_lambda) != one_temporal.end())
                temp_params[schema::json_temp_PL_lambda] = one_temporal[schema::json_temp_PL_lambda];
            if (one_temporal.find(schema::json_temp_Nor_mu) != one_temporal.end())
                temp_params[schema::json_temp_Nor_mu] = one_temporal[schema::json_temp_Nor_mu];
            if (one_temporal.find(schema::json_temp_Nor_sigma) != one_temporal.end())
                temp_params[schema::json_temp_Nor_sigma] = one_temporal[schema::json_temp_Nor_sigma];
        }

        // Out-Degree Distribution
        auto& out_dist = one_edge[schema::json_out_dist];
        // std::string o_dist_type = out_dist[schema::json_dist_type];
        st_one_edge.outd_type = out_dist[schema::json_dist_type];
        std::unordered_map<std::string, double>& out_params = st_one_edge.out_params;
        out_params[schema::json_dist_min_degree] = out_dist[schema::json_dist_min_degree];
        out_params[schema::json_dist_max_degree] = out_dist[schema::json_dist_max_degree];
        if (out_dist.find(schema::json_dist_PL_lambda) != out_dist.end())
            out_params[schema::json_dist_PL_lambda] = out_dist[schema::json_dist_PL_lambda];
        if (out_dist.find(schema::json_dist_Nor_mu) != out_dist.end())
            out_params[schema::json_dist_Nor_mu] = out_dist[schema::json_dist_Nor_mu];
        if (out_dist.find(schema::json_dist_Nor_sigma) != out_dist.end())
            out_params[schema::json_dist_Nor_sigma] = out_dist[schema::json_dist_Nor_sigma];

        // In-Degree Distribution
        auto& in_dist = one_edge[schema::json_in_dist];
        st_one_edge.ind_type = in_dist[schema::json_dist_type];
        std::unordered_map<std::string, double>& in_params = st_one_edge.in_params;
        in_params[schema::json_dist_min_degree] = in_dist[schema::json_dist_min_degree];
        in_params[schema::json_dist_max_degree] = in_dist[schema::json_dist_max_degree];
        if (in_dist.find(schema::json_dist_PL_lambda) != in_dist.end())
            in_params[schema::json_dist_PL_lambda] = in_dist[schema::json_dist_PL_lambda];
        if (in_dist.find(schema::json_dist_Nor_mu) != in_dist.end())
            in_params[schema::json_dist_Nor_mu] = in_dist[schema::json_dist_Nor_mu];
        if (in_dist.find(schema::json_dist_Nor_sigma) != in_dist.end())
            in_params[schema::json_dist_Nor_sigma] = in_dist[schema::json_dist_Nor_sigma];

        // Community Distribution (optional)
        st_one_edge.b_social = false;
        if (one_edge.find(schema::json_comm) != one_edge.end()) {
            st_one_edge.b_social = true;
            auto& comm = one_edge[schema::json_comm];
            std::unordered_map<std::string, double>& comm_params = st_one_edge.comm_params;
            comm_params[schema::json_comm_amount] = comm[schema::json_comm_amount];
            comm_params[schema::json_comm_lambda] = comm[schema::json_comm_lambda];
            comm_params[schema::json_comm_rho] = comm[schema::json_comm_rho];

            if (!st_one_edge.b_temporal) st_one_edge.b_social = true;
            // anchor communities
            else if (comm.find(schema::json_comm_window_size) != comm.end()) {
                comm_params[schema::json_comm_window_size] = comm[schema::json_comm_window_size];
                // comm_params[schema::json_comm_window_step] = one_comm[schema::json_comm_window_step];
                st_one_edge.b_social = true;
                // split time window
                std::unordered_map<std::string, double>& temp_params = st_one_edge.temp_params;
                st_one_edge.windSplit = Utility::splitWindow(
                    comm_params[schema::json_comm_amount],
                    comm_params[schema::json_comm_window_size],
                    temp_params[schema::json_temp_min_timestamp],
                    temp_params[schema::json_temp_max_timestamp]);
                st_one_edge.olAnchorComm = Utility::idenOlAnchorComm(comm_params[schema::json_comm_amount]);
            }
            // split community
            st_one_edge.commSplit = Utility::splitCommunity(s_nodes, t_nodes,
                comm_params[schema::json_comm_amount], comm_params[schema::json_comm_lambda]);
            // overlapping
            st_one_edge.b_overlap = false;
            if (comm.find(schema::json_comm_overlap) != comm.end()) {
                st_one_edge.b_overlap = true;
                st_one_edge.dv_overlap = comm[schema::json_comm_overlap];
                size_t n_comm = st_one_edge.commSplit.size();
                std::cout << "n_comm: " << n_comm << std::endl;
                int_t n_pair = n_comm / 2, i = 0;    // every community gets a overlapping anchor community on average
                while (i < n_pair) {
                    int_t a = rand.nextInt(n_comm - 1);
                    int_t b = rand.nextInt(n_comm - 1);
                    std::pair<int_t, int_t> one_pair(a, b);
                    if (one_pair.first == one_pair.second) continue;
                    if (st_one_edge.overlapComm[one_pair.first].insert(one_pair.second).second &&
                        st_one_edge.overlapComm[one_pair.second].insert(one_pair.first).second)
                        ++i;
                }
            }
        }

        // Embedded anchor communities (optional)
        st_one_edge.b_embedded = false;
        if (one_edge.find(schema::json_embd) != one_edge.end()) {
            st_one_edge.b_embedded = true;

            auto& embd = one_edge[schema::json_embd];
            std::unordered_map<std::string, double>& embd_comm_params = st_one_edge.embd_comm_params;
            embd_comm_params[schema::json_embd_window_size] = embd[schema::json_embd_window_size];
            embd_comm_params[schema::json_embd_srce_amount] = embd[schema::json_embd_srce_amount];
            embd_comm_params[schema::json_embd_trgt_amount] = embd[schema::json_embd_trgt_amount];
            embd_comm_params[schema::json_embd_edge_amount] = embd[schema::json_embd_edge_amount];
            embd_comm_params[schema::json_embd_comm_amount] = embd[schema::json_embd_comm_amount];
            embd_comm_params[schema::json_embd_comm_lambda] = embd[schema::json_embd_comm_lambda];
            embd_comm_params[schema::json_embd_comm_rho] = embd[schema::json_embd_comm_rho];

            auto& embd_temp = embd[schema::json_embd_temp];
            std::unordered_map<std::string, double>& embd_temp_params = st_one_edge.embd_temp_params;
            embd_temp_params[schema::json_temp_min_timestamp] = embd_temp[schema::json_temp_min_timestamp];
            embd_temp_params[schema::json_temp_max_timestamp] = embd_temp[schema::json_temp_max_timestamp];
            if (embd_temp.find(schema::json_temp_PL_lambda) != embd_temp.end())
                embd_temp_params[schema::json_temp_PL_lambda] = embd_temp[schema::json_temp_PL_lambda];
            if (embd_temp.find(schema::json_temp_Nor_mu) != embd_temp.end())
                embd_temp_params[schema::json_temp_Nor_mu] = embd_temp[schema::json_temp_Nor_mu];
            if (embd_temp.find(schema::json_temp_Nor_sigma) != embd_temp.end())
                embd_temp_params[schema::json_temp_Nor_sigma] = embd_temp[schema::json_temp_Nor_sigma];
            // split time window
            st_one_edge.embdWindSplit = Utility::splitWindow(
                embd_comm_params[schema::json_embd_comm_amount],
                embd_comm_params[schema::json_embd_window_size],
                embd_temp_params[schema::json_temp_min_timestamp],
                embd_temp_params[schema::json_temp_max_timestamp]);
            st_one_edge.embdOlAnchorComm = Utility::idenOlAnchorComm(embd_comm_params[schema::json_embd_comm_amount]);
        }
        std::cout << "[Generation::generate_plan] " << st_one_edge.e_label << ".b_temporal: " << st_one_edge.b_temporal << std::endl;
        std::cout << "[Generation::generate_plan] " << st_one_edge.e_label << ".b_social: " << st_one_edge.b_social<< std::endl;
        std::cout << "[Generation::generate_plan] " << st_one_edge.e_label << ".b_overlap: " << st_one_edge.b_overlap << std::endl;
        // add
        edge_gen_plan.push_back(st_one_edge);
    }
}

void Generation::run() {
    bool is_legal = check_json();
    if (!is_legal) {
        std::cerr << "[Generation::run] JSON format is wrong." << std::endl;
        return;
    }
    gb_start_gen = true;
    generate_plan();

    // Generate according to the plan
    // Node attribute
    // TODO

    // Topology & Attributes
    for (auto& st_one_edge : edge_gen_plan) {
        if (st_one_edge.b_temporal) {
            if (st_one_edge.b_social) temporalSocialGraph(st_one_edge);
            else temporalSimpleGraph(st_one_edge);
        } else {
            if (st_one_edge.b_social) {
                if (b_streaming_style) streamingSocialGraph(st_one_edge);
                else socialGraph(st_one_edge);
            } else {
                if (b_streaming_style) streamingSimpleGraph(st_one_edge);
                else simpleGraph(st_one_edge);
            }
        }

        if (st_one_edge.b_embedded) embeddedGraph(st_one_edge);
    }

    std::cout << "[Generation::run] end." << std::endl;
    gb_gen_done = true;
}

std::string Generation::currentGenerationTag() {
    return gp_tag;
}

double Generation::currentGenerationProgress() {
    return gp_progress;
}

bool Generation::isGenerationDone() {
    return gb_gen_done;
}

bool Generation::hasGenerationStart() {
    return gb_start_gen;
}

int_t Generation::getActualEdges(std::string e_label) {
    if (actual_edge_nums.count(e_label)) {
        return actual_edge_nums[e_label];
    }
    return 0;
}

void Generation::simpleGraph(St_EdgeGeneration& st_edge) {
    // according to st_edge
    std::unordered_map<std::string, double>& out_params = st_edge.out_params;
    std::unordered_map<std::string, double>& in_params = st_edge.in_params;
    std::string& ind_type = st_edge.ind_type;
    std::string& outd_type = st_edge.outd_type;
    int_t s_nodes = st_edge.s_nodes;
    int_t t_nodes = st_edge.t_nodes;
    int_t n_edges = st_edge.n_edges;
    std::string& filename = st_edge.filename;

    gp_tag = st_edge.e_label;
    std::cout << "[Generation::simpleGraph]: " << st_edge.e_source << " -> " << st_edge.e_target << std::endl;
    bool is_homo = (st_edge.e_source == st_edge.e_target);

    int_t id_min = in_params[schema::json_dist_min_degree];
    int_t id_max = in_params[schema::json_dist_max_degree];
    int_t od_min = out_params[schema::json_dist_min_degree];
    int_t od_max = out_params[schema::json_dist_max_degree];
    // id_max = std::max(id_max, (int_t)1);
    // od_max = std::max(od_max, (int_t)1);
    Distribution *out_dist = getDist(od_min, od_max, s_nodes, n_edges, out_params, true, outd_type);

#ifdef DEBUG
    std::cout << "[Generation::simpleGraph] Out distribution" << std::endl;
#endif

    Distribution *in_dist = getDist(id_min, id_max, t_nodes, n_edges, in_params, false, ind_type);

#ifdef DEBUG
    std::cout << "[Generation::simpleGraph] In distribution" << std::endl;
#endif

    // show process
    double cur = 0.0;
    double progress = 0.0;
    ProgressBar progress_bar;
    int_t actual_edges = 0;

#ifdef PARALLEL
    std::vector<Store*> store_list;
    for (int i = 0; i < n_threads; ++i) {
        std::string fn_thread = filename + "_" + std::to_string(i);
        store_list.push_back(new Store(fn_thread, g_enum_format));
    }
#else
    Store *store = new Store(filename, g_enum_format);
#endif

#ifdef PATCH_VPP
    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
    bool b_patch = false;
    Store *patch_store = nullptr;
    std::vector<Store*> patch_store_list;
    std::vector<std::string> url_list;

    if (st_edge.e_source == "Page" && st_edge.e_target == "VPerson") {
        b_patch = true;
        std::string pp_filename = filename + "_co_occurrence";

#ifdef PARALLEL
        for (int i = 0; i < n_threads; ++i) {
            std::string fn_thread = pp_filename + "_" + std::to_string(i);
            patch_store_list.push_back(new Store(fn_thread, g_enum_format));
        }
#else
        patch_store = new Store(pp_filename, g_enum_format);
#endif

        std::ifstream fin("urls.txt");
        if (!fin.is_open()) {
            std::cout << "Cannot open urls.txt" << std::endl;
        }
        std::string a_url;
        while (std::getline(fin, a_url)) {
            url_list.push_back(a_url);
        }
    }
#endif
    // END PATCH

    // parallel
#ifdef PARALLEL
    #pragma omp parallel for schedule (dynamic, thread_chunk_size)
#endif
    for (int_t i = 0; i < s_nodes; ++i) {
        Store *store_ptr = nullptr;
#ifdef PATCH_VPP
        Store *patch_store_ptr = nullptr;
#endif

#ifdef PARALLEL
        int tid = omp_get_thread_num();
        store_ptr = store_list[tid];
#ifdef PATCH_VPP
        patch_store_ptr = patch_store_list[tid];
#endif
#else
        store_ptr = store;
#ifdef PATCH_VPP
        patch_store_ptr = patch_store;
#endif
#endif

        // 各线程独有
        std::unordered_set<int_t> nbrs;

        int_t out_degree = out_dist->genOutDegree(i);
        for (int_t j = 0; j < out_degree; ++j) {
            int_t t = in_dist->genTargetID();
            while (is_homo && t == i) {
                t = in_dist->genTargetID();
            }

            // PATCH, Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
            if (b_patch) {
                std::string temp = url_list[i] + "\tvp_" + std::to_string(t) + "_VPerson\tvp_";
                for (auto one : nbrs) {
                    std::string attach = temp + std::to_string(one) + "_VPerson\tCo_occurRelation";
                    patch_store_ptr->writeTSVLine(t, one, attach);
                }
            }
#endif
            // END PATCH

            nbrs.insert(t);
        }

#ifdef PARALLEL
        #pragma omp atomic
#endif
        actual_edges += nbrs.size();

        // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
        if (b_patch) {
            for (auto one : nbrs) {
                std::string attach = "vp_" + std::to_string(one) + "_VPerson\tp_" + std::to_string(i) + "_Page\tOccurRelation";
                store_ptr->writeTSVLine(one, i, attach);
            }
        } else {
            store_ptr->writeLine(i, nbrs);
        }
#else
        store_ptr->writeLine(i, nbrs);
#endif
        // END PATCH

        nbrs.clear();
        // if (actual_edges > n_edges) {
        //     int_t stop_degree = i;
        //     std::cout << "[Generation::simpleGraph] Stop at " + stop_degree << std::endl;
        //     break;
        // }

        cur = (double)actual_edges / (double)n_edges;
        if (cur - progress >= 0.01) {
#ifdef PARALLEL
            #pragma omp atomic
#endif
            progress += 0.01;
            gp_progress = progress;
            // showProgress();
            progress_bar.set_progress(progress);
        }
    }

    // Store close
#ifdef PARALLEL
    for (int i = 0; i < n_threads; ++i) {
        store_list[i]->close();
    }

    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
    if (b_patch) {
        for (int i = 0; i < n_threads; ++i) {
            patch_store_list[i]->close();
        }
    }
#endif
    // END PATCH

#else
    store->close();

    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
    if (b_patch) {
        patch_store->close();
    }
#endif
    // END PATCH

#endif

    gp_progress = 1.0;
    std::cout << "[Generation::simpleGraph] #Source Nodes = " << s_nodes << ", #Target Nodes = " << t_nodes << std::endl;
    std::cout << "[Generation::simpleGraph] #Actual Edges = " << actual_edges << std::endl;
    std::cout << "[Generation::simpleGraph] #Expect Edges = " << n_edges << std::endl;

    actual_edge_nums[st_edge.e_label] = actual_edges;
}

void Generation::socialGraph(St_EdgeGeneration& st_edge) {
    // according to st_edge
    std::unordered_map<std::string, double>& out_params = st_edge.out_params;
    std::unordered_map<std::string, double>& in_params = st_edge.in_params;
    std::unordered_map<std::string, double>& comm_params = st_edge.comm_params;
    std::string& ind_type = st_edge.ind_type;
    std::string& outd_type = st_edge.outd_type;
    int_t s_nodes = st_edge.s_nodes;
    int_t t_nodes = st_edge.t_nodes;
    int_t n_edges = st_edge.n_edges;
    std::string& filename = st_edge.filename;

    // start information
    // gp_tag = "Edge-social" + st_edge.e_source + "-" + st_edge.e_target;
    gp_tag = st_edge.e_label;
    std::cout << "[Generation::socialGraph]: " << st_edge.e_source << " -> " << st_edge.e_target << std::endl;
    bool is_homo = (st_edge.e_source == st_edge.e_target);

    int_t actual_edges = 0;
    int_t extra_edges = 0;
    /*int_t actual_edges_v = 0;
    int_t extra_edges_v = 0;
    int_t overlap_edges_v = 0;*/

#ifdef PARALLEL
    std::vector<Store*> store_list;
    for (int i = 0; i < n_threads; ++i) {
        std::string fn_thread = filename + "_" + std::to_string(i);
        store_list.push_back(new Store(fn_thread, g_enum_format));
    }
#else
    Store *store = new Store(filename, g_enum_format);
#endif

    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)    
#ifdef PATCH_VPP
    bool b_patch = false;
    if (st_edge.e_source == "VPerson" && st_edge.e_target == "VPerson") {
        b_patch = true;
    }
#endif
    // END PATCH

    int_t n_comm = comm_params[schema::json_comm_amount];
    double comm_lambda = comm_params[schema::json_comm_lambda];
    double comm_rho = comm_params[schema::json_comm_rho];

#ifdef DEBUG
    std::cout << "[Generation::socialGraph] Before splitting ..." << std::endl;
#endif

    std::cout << "#Threads: "<<n_threads<<std::endl;

    std::vector<std::vector<int_t>>& split = st_edge.commSplit;
    int an_comms = split.size();

    // // Write `Community Split`
    // std::string comm_fn = filename + "_comm";
    // std::ofstream fout(comm_fn.c_str());
    // if (!fout.is_open()) {
    //     std::cerr << "[Generation::socialGraph] Cannot open " << comm_fn << std::endl;
    // }
    // int_t sp_comm_rid = 0;
    // int_t sp_comm_cid = 0;
    // fout << sp_comm_rid << "\t" << sp_comm_cid << std::endl;
    // for (auto& row : split) {
    //     sp_comm_rid += row[0];
    //     sp_comm_cid += row[1];
    //     fout << sp_comm_rid << "\t" << sp_comm_cid << std::endl;
    // }
    // fout.close();

    // // for grid parallel version
    // std::vector<int_t> split_id(an_comms, 0);
    // if (an_comms > 0) {
    //     for (int i = 1; i < an_comms; ++i) {
    //         split_id[i] = split_id[i - 1] + split[i - 1][0];
    //     }
    // }

    // for original parallel version
    std::vector<int_t> sp_row_id(s_nodes, 0);
    std::vector<int_t> cumu_row_id(s_nodes, 0);
    int_t gi = 0, sri = 0, cri = 0;
    for (int i = 0; i < an_comms; ++i) {
        for (int_t j = 0; j < split[i][0]; ++j) {
            sp_row_id[gi] = i;
            cumu_row_id[gi] = j;
            gi ++;
        }
    }

    std::cout << "[Generation::socialGraph] #Communities = " << an_comms << std::endl;
    for (int i = 0; i < an_comms; ++i)
        std::cout << split[i][0] << ", ";
    std::cout << std::endl;

#ifdef DEBUG 
    std::cout << "[Generation::socialGraph] #Communities = " << an_comms << std::endl;
    for (int i = 0; i < an_comms; ++i)
        std::cout << split[i][0] << " , " << split[i][1] << std::endl;
#endif

    int_t id_min = in_params[schema::json_dist_min_degree];
    int_t id_max = in_params[schema::json_dist_max_degree];
    int_t od_min = out_params[schema::json_dist_min_degree];
    int_t od_max = out_params[schema::json_dist_max_degree];
    std::unordered_map<int, Distribution*> row_dist;
    std::unordered_map<int, Distribution*> col_dist;
    for (int i = 0; i < an_comms; ++i) {
        if (!row_dist.count(split[i][0])) {
            int_t sub_edges = Utility::mathRound(split[i][0] * 1.0 / s_nodes * n_edges);
            row_dist[split[i][0]] = getDist(od_min, od_max, split[i][0], sub_edges, out_params, true, outd_type);
        }
        if (!col_dist.count(split[i][1])) {
            int_t sub_edges = Utility::mathRound(split[i][1] * 1.0 / t_nodes * n_edges);
            col_dist[split[i][1]] = getDist(id_min, id_max, split[i][1], sub_edges, in_params, false, ind_type);
        }
    }

    int_t cumu_row = 0;
    int_t sp_row_i = 0;

    // show process
    double cur = 0.0;
    double progress = 0.0;
    ProgressBar progress_bar;

    // generate START
#ifdef PARALLEL
    #pragma omp parallel for schedule (dynamic, thread_chunk_size)
#endif
    for (int_t i = 0; i < s_nodes; ++i) {
        Store *store_ptr = nullptr;
#ifdef PARALLEL
        int tid = omp_get_thread_num();
        store_ptr = store_list[tid];
#else
        store_ptr = store;
#endif

        // cumu_row ++;
        // if (cumu_row > split[sp_row_i][0]) {
        //     sp_row_i ++;
        //     if (sp_row_i >= an_comms)
        //         sp_row_i = an_comms - 1;
        //     cumu_row = 1;
        // }
        cumu_row = cumu_row_id[i];
        sp_row_i = sp_row_id[i];

        Distribution *o_dist = row_dist[split[sp_row_i][0]];
        int_t main_out_degree = o_dist->genOutDegree(cumu_row);
        int_t extra_out_degree = 0;
        if (rand.nextReal() < comm_rho)
            extra_out_degree = extraDegree(od_max - main_out_degree + 10, comm_rho + 1.0);

        int_t cumu_col = 0;
        int_t sp_col_j = 0;
        std::unordered_set<int_t> nbrs;
        while (sp_col_j < an_comms) {
            int_t num;
            if (sp_col_j == sp_row_i) { // 社区内的出度
                num = main_out_degree;
            } else {    // 社区间的出度
                num = Utility::mathRound(extra_out_degree * 1.0 * split[sp_col_j][1] / (1.0 * (t_nodes - split[sp_row_i][1])));
                // num = Utility::mathRound(extra_out_degree * 1.0 * split[sp_col_j][1] / (1.0 * t_nodes));
                // num = extra_out_degree_j;
            }
            // if(nbrs.size()>0){
            //     std::cout <<"nbrs.size: "<< nbrs.size() <<std::endl;
            // }

            /*int_t within_edges_ = 0;
            int_t extra_edges_ = 0;
            int_t overlap_edges_ = 0;*/
            // overlapping (new version)
            if (sp_col_j == sp_row_i) {
                Distribution *i_dist = col_dist[split[sp_col_j][1]];
                // while (nbrs.size() < num) {
                for (int_t xp = 0; xp < num; ++xp) {
                    int_t t = i_dist->genTargetID();
                    nbrs.insert(t + cumu_col);
                }

                //within_edges_ = nbrs.size();
            } else {
                // if (num) std::cout << "i: " << i << ", sp_col_j: " << sp_col_j << ", num: " << num << std::endl;
                // if (num>0){
                //     std::cout <<"num: "<<num<<std::endl;
                // }
                int_t size = split[sp_col_j][1];
                while (nbrs.size() < num) {
                    int_t t = rand.nextInt(size);
                    nbrs.insert(t + cumu_col);
                }
                //extra_edges_ = nbrs.size();
                // if(nbrs.size()>0){
                //     std::cout <<"nbrs.size: "<< nbrs.size() <<std::endl;
                // }

                bool flag = false;
                if (st_edge.b_overlap &&
                    st_edge.overlapComm.count(sp_row_i) > 0 &&
                    st_edge.overlapComm[sp_row_i].count(sp_col_j) > 0) {
                    flag = true;
                }
                if (flag && sp_row_i < sp_col_j) {
                    double ol = st_edge.dv_overlap;
                    int_t thre_row_i = (int_t)(split[sp_row_i][0] * ol);
                    if (cumu_row + 1 >= thre_row_i) {
                        int_t ol_num = num * 10 * ol;
                        for (int_t ti = 0; ti < ol_num; ++ ti) {
                            int_t t = rand.nextInt(size);
                            nbrs.insert(t + cumu_col);
                        }
                    }
                }
                // if(nbrs.size()>0){
                //     std::cout <<"nbrs.size: "<< nbrs.size() <<std::endl;
                // }
                if (flag && sp_row_i > sp_col_j) {
                    double ol = st_edge.dv_overlap;
                    int_t sp_size = (int_t)(size * ol);
                    int_t ol_size = size - sp_size;
                    int_t ol_num = num * 10 * (1.0 - ol);
                    for (int_t ti = 0; ti < ol_num; ++ti) {
                        int_t t = rand.nextInt(ol_size);
                        nbrs.insert(t + cumu_col + sp_size);
                    }
                }
                // if(nbrs.size()>0){
                //     std::cout <<"nbrs.size: "<< nbrs.size() <<std::endl;
                // }
                // std::cout <<"extra_edges: "<< nbrs.size() <<std::endl;
                //overlap_edges_ = nbrs.size() - extra_edges_;
            }

            if (is_homo && nbrs.count(i)) {
                nbrs.erase(i);
            }

#ifdef PARALLEL
            #pragma omp atomic
#endif
            actual_edges += nbrs.size();
            if (sp_col_j != sp_row_i) {
                // if(nbrs.size()>0){
                //     std::cout <<"nbrs.size: "<< nbrs.size() <<std::endl;
                // }
#ifdef PARALLEL
                #pragma omp atomic
#endif
                extra_edges += nbrs.size();
            }

            /*actual_edges_v += within_edges_ + extra_edges_ + overlap_edges_;
            extra_edges_v += extra_edges_;
            overlap_edges_v += overlap_edges_;*/
            
            // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
            if (b_patch) {
                for (auto n : nbrs) {
                    int_t ri = rand.nextInt(patch_contact_list.size() - 1);
                    std::string attach = patch_contact_list[ri] + "\tvp_" + std::to_string(i) + "_VPerson\tvp_" + std::to_string(n) + "_VPerson\tContactRelation";
                    store_ptr->writeTSVLine(i, n, attach);
                }
            } else {
                store_ptr->writeLine(i, nbrs);
            }
#else
            store_ptr->writeLine(i, nbrs);
#endif
            // END PATCH

            nbrs.clear();
            cumu_col += split[sp_col_j][1];
            sp_col_j ++;
        }

        cur = (double)actual_edges / (double) n_edges;
        if (cur - progress >= 0.01) {
#ifdef PARALLEL
            #pragma omp atomic
#endif
            progress += 0.01;
            gp_progress = progress;
            // progress_bar.set_progress(progress);
        }
    }
    // generate END

    // // parallel generation START, another version, grid split
    // #pragma omp parallel for schedule (dynamic, 1)
    // for (sp_row_i = 0; sp_row_i < an_comms; ++ sp_row_i) {
    //     int_t row_start_id = split_id[i];
    //     int_t row_end_id = row_start_id + split[sp_row_i][0];

    //     // variables in this grid
    //     Distribution *o_dist = row_dist[split[sp_row_i][0]];

    //     for (int_t i = row_start_id; i < row_end_id; ++i) {
    //         cumu_row = i - row_start_id;
    //         int_t main_out_degree = o_dist->genOutDegree(cumu_row);
    //         int_t extra_out_degree = 0;
    //         if (rand.nextReal() < comm_rho)
    //             extra_out_degree = extraDegree(od_max - main_out_degree + 10, comm_rho + 1.0);

    //         // generate
    //         cumu_col = 0;
    //         sp_col_j = 0;
    //         while (sp_col_j < an_comms) {
    //             int_t num;
    //             if (sp_col_j == sp_row_i) {
    //                 num = main_out_degree;
    //             } else {
    //                 num = Utility::mathRound(extra_out_degree * 1.0 * split[sp_col_j][1] / (1.0 * (t_nodes - split[sp_row_i][1])));
    //             }

    //             // overlapping (new version)
    //             if (sp_col_j == sp_row_i) {
    //                 Distribution *i_dist = col_dist[split[sp_col_j][1]];
    //                 while (nbrs.size() < num) {
    //                     int_t t = i_dist->genTargetID();
    //                     nbrs.insert(t + cumu_col);
    //                 }
    //             } else {
    //                 int_t size = split[sp_col_j][1];
    //                 // uniform
    //                 while (nbrs.size() < num) {
    //                     int_t t = rand.nextInt(size);
    //                     nbrs.insert(t + cumu_col);
    //                 }
    //                 // extra :+
    //                 bool flag = false;
    //                 if (st_edge.b_overlap &&
    //                     st_edge.overlapComm.count(sp_row_i) > 0 &&
    //                     st_edge.overlapComm[sp_row_i].count(sp_col_j) > 0) {
    //                     flag = true;
    //                 }
    //                 if (flag && sp_row_i < sp_col_j) {
    //                     double ol = st_edge.dv_overlap;
    //                     int_t thre_row_i = (int_t)(split[sp_row_i][0] * ol);
    //                     if (cumu_row >= thre_row_i) {
    //                         int_t ol_num = num * 10 * ol;
    //                         for (int_t ti = 0; ti < ol_num; ++ ti) {
    //                             int_t t = rand.nextInt(size);
    //                             nbrs.insert(t + cumu_col);
    //                         }
    //                     }
    //                 }
    //                 if (flag && sp_row_i > sp_col_j) {
    //                     double ol = st_edge.dv_overlap;
    //                     int_t sp_size = (int_t)(size * ol);
    //                     int_t ol_size = size - sp_size;
    //                     int_t ol_num = num * 10 * (1.0 - ol);
    //                     for (int_t ti = 0; ti < ol_num; ++ti) {
    //                         int_t t = rand.nextInt(ol_size);
    //                         nbrs.insert(t + cumu_col + sp_size);
    //                     }
    //                 }
    //             }

    //             actual_edges += nbrs.size();
    //             if (sp_col_j != sp_row_i)
    //                 extra_edges += nbrs.size();
    //             store->writeLine(i, nbrs);

    //             nbrs.clear();
    //             cumu_col += split[sp_col_j][1];
    //             sp_col_j ++;
    //         } //! while col

    //         cur = (double)actual_edges / (double) n_edges;
    //         if (cur - progress >= 0.01) {
    //             #pragma omp atomic
    //             progress += 0.01;
    //             progress_bar.set_progress(progress);
    //         }
    //     } //! for row grid
    // }
    // // parallel generation END

    // Store close
#ifdef PARALLEL
    for (int i = 0; i < n_threads; ++i) {
        store_list[i]->close();
    }
#else
    store->close();
#endif

    gp_progress = 1.0;
    // Output info
    std::cout << "[Generation::socialGraph] #Source Nodes = " << s_nodes << " , #Target Nodes = " << t_nodes << std::endl;
    std::cout << "[Generation::socialGraph] #Actual Edges = " << actual_edges << std::endl;
    std::cout << "[Generation::socialGraph] #Extra Edges = " << extra_edges << std::endl;
    /*std::cout << "[Generation::socialGraph] #Actual Edges V = " << actual_edges_v << std::endl;
    std::cout << "[Generation::socialGraph] #Extra Edges V = " << extra_edges_v << std::endl;
    std::cout << "[Generation::socialGraph] #Overlap Edges V = " << overlap_edges_v << std::endl;*/
    std::cout << "[Generation::socialGraph] #Expect Edges = " << n_edges << std::endl;

    actual_edge_nums[st_edge.e_label] = actual_edges;
}

void Generation::streamingSimpleGraph(St_EdgeGeneration& st_edge) {
    // according to st_one_edge
    std::unordered_map<std::string, double>& out_params = st_edge.out_params;
    std::unordered_map<std::string, double>& in_params = st_edge.in_params;
    std::string& ind_type = st_edge.ind_type;
    std::string& outd_type = st_edge.outd_type;
    int_t s_nodes = st_edge.s_nodes;
    int_t t_nodes = st_edge.t_nodes;
    int_t n_edges = st_edge.n_edges;
    std::string& basename = st_edge.basename;
    std::string& format = g_format;
    double gr = g_gr;

    // gp_tag = "Edge-streaming" + st_edge.e_source + "-" + st_edge.e_target;
    gp_tag = st_edge.e_label;
    std::cout << "[Generation::streamingSimpleGraph]: " << st_edge.e_source << " -> " << st_edge.e_target << std::endl;

    int_t id_min = in_params[schema::json_dist_min_degree];
    int_t id_max = in_params[schema::json_dist_max_degree];
    int_t od_min = out_params[schema::json_dist_min_degree];
    int_t od_max = out_params[schema::json_dist_max_degree];

#ifdef DEBUG
    std::cout << "[Generation::streamingSimpleGraph] Out distribution" << std::endl;
#endif

    Distribution *in_dist = getDist(id_min, id_max, t_nodes, n_edges, in_params, false, ind_type);

    // show process
    double cur = 0.0;
    double progress = 0.0;
    ProgressBar progress_bar;

    int_t actual_edges = 0;
    int_t iter_edges = 0;
    int file_no = 0;

    std::string filename = basename + "_" + std::to_string(file_no) + "." + format;
    Store *store = new Store(filename, g_enum_format);
    std::unordered_set<int_t> nbrs;
    double cur_rate = gr;
    OutDegreeDistribution *pre_odd = nullptr;
    do {
        iter_edges = 0;
        int_t involed_n = std::min((int_t)(cur_rate * s_nodes), s_nodes);
        int_t involed_m = std::min((int_t)(cur_rate * n_edges), n_edges);
        // int_t start_node_id = s_nodes - involed_n;
        OutDegreeDistribution *cur_odd = getOutDist(od_min, od_max, involed_n, involed_m, out_params, outd_type);
        DeltaOutDistribution *dod = new DeltaOutDistribution(pre_odd, cur_odd);
        dod->set_bucket(2);

        for (int_t i = 0; i < involed_n; ++i) {
            int_t out_degree = dod->get_out_degree(i);
            // while (nbrs.size() < out_degree) {
            for (int_t xp = 0; xp < out_degree; ++xp) {
                int_t t = in_dist->genTargetID();
                // if (t < start_node_id)
                if (t < involed_n)
                    continue;
                nbrs.insert(t);
            }
            actual_edges += nbrs.size();
            iter_edges += nbrs.size();
            // int_t node_id = start_node_id + i;
            int_t node_id = i;
            store->writeLine(node_id, nbrs);
            nbrs.clear();

            cur = (double)i / (double)s_nodes;
            if (cur - progress >= 0.01) {
                progress += 0.01;
                gp_progress = progress;
                progress_bar.set_progress(progress);
            }
        }

        // close
        store->flush();
        store->close();

#ifdef DEBUG
        std::cout << "[Generation::streamingSimpleGraph] Stream Iteration " << file_no << std::endl;
#endif

        file_no ++;
        cur_rate += gr;
        pre_odd = cur_odd;
        filename = basename + "_" + std::to_string(file_no) + "." + format;
        if (cur_rate < 1.0) {
            store = new Store(filename, g_enum_format);
        }
    } while (cur_rate < 1.0);
    // store->close();

    // Generation end
    gp_progress = 1.0;
    std::cout << "[Generation::streamingSimpleGraph] #Source Nodes = " << s_nodes << ", #Target Nodes = " << t_nodes << std::endl;
    std::cout << "[Generation::streamingSimpleGraph] #Actual Edges = " << actual_edges << std::endl;
    std::cout << "[Generation::streamingSimpleGraph] #Expect Edges = " << n_edges << std::endl; 

    actual_edge_nums[st_edge.e_label] = actual_edges;
}

void Generation::streamingSocialGraph(St_EdgeGeneration& st_edge) {
    // according to st_edge
    std::unordered_map<std::string, double>& out_params = st_edge.out_params;
    std::unordered_map<std::string, double>& in_params = st_edge.in_params;
    std::unordered_map<std::string, double>& comm_params = st_edge.comm_params;
    std::string& ind_type = st_edge.ind_type;
    std::string& outd_type = st_edge.outd_type;
    int_t s_nodes = st_edge.s_nodes;
    int_t t_nodes = st_edge.t_nodes;
    int_t n_edges = st_edge.n_edges;
    std::string& basename = st_edge.basename;
    std::string& format = g_format;
    double gr = g_gr;

    std::cout << "[Generation::streamingSocialGraph]: " << st_edge.e_source << " -> " << st_edge.e_target << std::endl;

    // TODO
}

void Generation::temporalSimpleGraph(St_EdgeGeneration& st_edge) {
    // according to st_edge
    std::unordered_map<std::string, double>& out_params = st_edge.out_params;
    std::unordered_map<std::string, double>& in_params = st_edge.in_params;
    std::string& ind_type = st_edge.ind_type;
    std::string& outd_type = st_edge.outd_type;
    int_t s_nodes = st_edge.s_nodes;
    int_t t_nodes = st_edge.t_nodes;
    int_t n_edges = st_edge.n_edges;
    std::string& filename = st_edge.filename;

    std::unordered_map<std::string, double>& temp_params = st_edge.temp_params;
    std::string& temp_type = st_edge.temp_type;

    gp_tag = st_edge.e_label;
    std::cout << "[Generation::temporalSimpleGraph]: " << st_edge.e_source << " -> " << st_edge.e_target << std::endl;
    bool is_homo = (st_edge.e_source == st_edge.e_target);

    int_t id_min = in_params[schema::json_dist_min_degree];
    int_t id_max = in_params[schema::json_dist_max_degree];
    int_t od_min = out_params[schema::json_dist_min_degree];
    int_t od_max = out_params[schema::json_dist_max_degree];

    Distribution *out_dist = getDist(od_min, od_max, s_nodes, n_edges, out_params, true, outd_type);

#ifdef DEBUG
    std::cout << "[Generation::temporalSimpleGraph] Out distribution" << std::endl;
#endif

    Distribution *in_dist = getDist(id_min, id_max, t_nodes, n_edges, in_params, false, ind_type);

#ifdef DEBUG
    std::cout << "[Generation::temporalSimpleGraph] In distribution" << std::endl;
#endif
    
    int_t ts_min = temp_params[schema::json_temp_min_timestamp];
    int_t ts_max = temp_params[schema::json_temp_max_timestamp];

    //Timestamp timer(ts_min, ts_max, temp_params);
    Timestamp* timer = getTimer(ts_min, ts_max, temp_params, temp_type);

#ifdef DEBUG
    std::cout << "[Generation::temporalSimpleGraph] Temporal timestamp" << std::endl;
#endif

    // show process
    double cur = 0.0;
    double progress = 0.0;
    ProgressBar progress_bar;
    int_t actual_edges = 0;

#ifdef PARALLEL
    std::vector<Store*> store_list;
    for (int i = 0; i < n_threads; ++i) {
        std::string fn_thread = filename + "_" + std::to_string(i);
        store_list.push_back(new Store(fn_thread, g_enum_format));
    }
#else
    Store *store = new Store(filename, g_enum_format);
#endif

#ifdef PATCH_VPP
    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
    bool b_patch = false;
    Store *patch_store = nullptr;
    std::vector<Store*> patch_store_list;
    std::vector<std::string> url_list;

    if (st_edge.e_source == "Page" && st_edge.e_target == "VPerson") {
        b_patch = true;
        std::string pp_filename = filename + "_co_occurrence";

#ifdef PARALLEL
        for (int i = 0; i < n_threads; ++i) {
            std::string fn_thread = pp_filename + "_" + std::to_string(i);
            patch_store_list.push_back(new Store(fn_thread, g_enum_format));
        }
#else
        patch_store = new Store(pp_filename, g_enum_format);
#endif

        std::ifstream fin("urls.txt");
        if (!fin.is_open()) {
            std::cout << "Cannot open urls.txt" << std::endl;
        }
        std::string a_url;
        while (std::getline(fin, a_url)) {
            url_list.push_back(a_url);
        }
    }
#endif
    // END PATCH

    // parallel FOR EACH NODE
#ifdef PARALLEL
    #pragma omp parallel for schedule (dynamic, thread_chunk_size)
#endif
    for (int_t i = 0; i < s_nodes; ++i) {
        Store *store_ptr = nullptr;
#ifdef PATCH_VPP
        Store *patch_store_ptr = nullptr;
#endif

#ifdef PARALLEL
        int tid = omp_get_thread_num();
        store_ptr = store_list[tid];
#ifdef PATCH_VPP
        patch_store_ptr = patch_store_list[tid];
#endif
#else
        store_ptr = store;
#ifdef PATCH_VPP
        patch_store_ptr = patch_store;
#endif
#endif

        // unique to each thread
        int_t out_degree = out_dist->genOutDegree(i);
        std::set<std::pair<int_t, int_t>> nbrs;
        for (int_t j = 0; j < out_degree; ) {
            // one neighbor of node i
            int_t nbr = in_dist->genTargetID();
            while (is_homo && nbr == i) { nbr = in_dist->genTargetID(); }
            int_t ts = timer->genTimestamp();

            // PATCH, Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
            if (b_patch) {
                std::string temp = url_list[i] + "\tvp_" + std::to_string(t) + "_VPerson\tvp_";
                for (auto one : nbrs) {
                    std::string attach = temp + std::to_string(one) + "_VPerson\tCo_occurRelation";
                    patch_store_ptr->writeTSVLine(nbr, one, attach);
                }
            }
#endif
            // END PATCH

            if (nbrs.insert({nbr, ts}).second)  // succeed
                j++;
        }

#ifdef PARALLEL
        #pragma omp atomic
#endif
        actual_edges += nbrs.size();

        // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
        if (b_patch) {
            for (auto one : nbrs) {
                std::string attach = "vp_" + std::to_string(one) + "_VPerson\tp_" + std::to_string(i) + "_Page\tOccurRelation";
                store_ptr->writeTSVLine(one, i, attach);
            }
        } else {
            store_ptr->writeLine(i, nbrs, tss);
        }
#else
        store_ptr->writeLine(i, nbrs);
#endif
        // END PATCH

        nbrs.clear();
        cur = (double)actual_edges / (double)n_edges;
        if (cur - progress >= 0.01) {
#ifdef PARALLEL
            #pragma omp atomic
#endif
            progress += 0.01;
            gp_progress = progress;
            // showProgress();
            progress_bar.set_progress(progress);
        }
    }

    // Store close
#ifdef PARALLEL
    for (int i = 0; i < n_threads; ++i) {
        store_list[i]->close();
    }

    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
    if (b_patch) {
        for (int i = 0; i < n_threads; ++i) {
            patch_store_list[i]->close();
        }
    }
#endif
    // END PATCH

#else
    store->close();

    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
    if (b_patch) {
        patch_store->close();
    }
#endif
    // END PATCH

#endif

    gp_progress = 1.0;
    std::cout << "[Generation::temporalSimpleGraph] #Source Nodes = " << s_nodes << ", #Target Nodes = " << t_nodes << std::endl;
    std::cout << "[Generation::temporalSimpleGraph] #Actual Edges = " << actual_edges << std::endl;
    std::cout << "[Generation::temporalSimpleGraph] #Expect Edges = " << n_edges << std::endl;

    actual_edge_nums[st_edge.e_label] = actual_edges;
}

void Generation::temporalSocialGraph(St_EdgeGeneration& st_edge) {
    // according to st_edge
    std::unordered_map<std::string, double>& out_params = st_edge.out_params;
    std::unordered_map<std::string, double>& in_params = st_edge.in_params;
    std::unordered_map<std::string, double>& comm_params = st_edge.comm_params;
    std::string& ind_type = st_edge.ind_type;
    std::string& outd_type = st_edge.outd_type;
    int_t s_nodes = st_edge.s_nodes;
    int_t t_nodes = st_edge.t_nodes;
    int_t n_edges = st_edge.n_edges;
    std::string& filename = st_edge.filename;

    std::unordered_map<std::string, double>& temp_params = st_edge.temp_params;
    std::string& temp_type = st_edge.temp_type;

    std::vector<std::vector<int_t>>& windSplit = st_edge.windSplit;
    std::vector<std::unordered_map<int_t, double>>& olAnchorComm = st_edge.olAnchorComm;

    // start information
    // gp_tag = "Edge-social" + st_edge.e_source + "-" + st_edge.e_target;
    gp_tag = st_edge.e_label;
    std::cout << "[Generation::temporalSocialGraph]: " << st_edge.e_source << " -> " << st_edge.e_target << std::endl;
    bool is_homo = (st_edge.e_source == st_edge.e_target);

    int_t actual_edges = 0;
    int_t extra_edges = 0;

#ifdef PARALLEL
    std::vector<Store*> store_list;
    for (int i = 0; i < n_threads; ++i) {
        std::string fn_thread = filename + "_" + std::to_string(i);
        store_list.push_back(new Store(fn_thread, g_enum_format));
    }
#else
    Store *store = new Store(filename, g_enum_format);
#endif

    // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)    
#ifdef PATCH_VPP
    bool b_patch = false;
    if (st_edge.e_source == "VPerson" && st_edge.e_target == "VPerson") {
        b_patch = true;
    }
#endif
    // END PATCH

    int_t n_comm = comm_params[schema::json_comm_amount];
    double comm_lambda = comm_params[schema::json_comm_lambda];
    double comm_rho = comm_params[schema::json_comm_rho];

#ifdef DEBUG
    std::cout << "[Generation::temporalSocialGraph] Before splitting ..." << std::endl;
#endif

    std::vector<std::vector<int_t>>& split = st_edge.commSplit;
    int an_comms = split.size();

    // for original parallel version
    std::vector<int_t> sp_row_id(s_nodes, 0);
    std::vector<int_t> cumu_row_id(s_nodes, 0);
    std::vector<int_t> cumu_col_psum(an_comms, 0);
    int_t gi = 0, sri = 0, cri = 0, psum = 0;
    for (int i = 0; i < an_comms; ++i) {
        cumu_col_psum[i] = psum;
        psum += split[i][1];
        for (int_t j = 0; j < split[i][0]; ++j) {
            sp_row_id[gi] = i;
            cumu_row_id[gi] = j;
            gi ++;
        }
    }

#ifdef DEBUG
    std::cout << "[Generation::temporalSocialGraph] #Communities = " << an_comms << std::endl;
    for (int i = 0; i < an_comms; ++i)
        std::cout << split[i][0] << " , " << split[i][1] << std::endl;
#endif

    int_t id_min = in_params[schema::json_dist_min_degree];
    int_t id_max = in_params[schema::json_dist_max_degree];
    int_t od_min = out_params[schema::json_dist_min_degree];
    int_t od_max = out_params[schema::json_dist_max_degree];
    std::unordered_map<int, Distribution*> row_dist;
    std::unordered_map<int, Distribution*> col_dist;
    for (int i = 0; i < an_comms; ++i) {
        if (!row_dist.count(split[i][0])) {
            int_t sub_edges = Utility::mathRound(split[i][0] * 1.0 / s_nodes * n_edges);
            row_dist[split[i][0]] = getDist(od_min, od_max, split[i][0], sub_edges, out_params, true, outd_type);
        }
        if (!col_dist.count(split[i][1])) {
            int_t sub_edges = Utility::mathRound(split[i][1] * 1.0 / t_nodes * n_edges);
            col_dist[split[i][1]] = getDist(id_min, id_max, split[i][1], sub_edges, in_params, false, ind_type);
        }
    }

    int_t ts_min = temp_params[schema::json_temp_min_timestamp];
    int_t ts_max = temp_params[schema::json_temp_max_timestamp];
    std::vector<Timestamp*> main_timer(an_comms);
    for (int i = 0; i < an_comms; ++i) { main_timer[i] = getTimer(windSplit[i][0], windSplit[i][1], temp_params, temp_type); }
    Timestamp* extra_timer = getTimer(ts_min, ts_max, std::unordered_map<std::string, double>(), schema::json_temp_Uniform);

    // show process
    double cur = 0.0;
    double progress = 0.0;
    ProgressBar progress_bar;

    // generate START
#ifdef PARALLEL
    #pragma omp parallel for schedule (dynamic, thread_chunk_size)
#endif
    for (int_t i = 0; i < s_nodes; ++i) {
        Store *store_ptr = nullptr;
#ifdef PARALLEL
        int tid = omp_get_thread_num();
        store_ptr = store_list[tid];
#else
        store_ptr = store;
#endif
        // unique to each thread
        int_t cumu_row_id_i = cumu_row_id[i];
        int_t sp_row_i = sp_row_id[i];
        int_t size_src_i = split[sp_row_i][0];
        int_t size_trg_i = split[sp_row_i][1];

        Distribution *o_dist = row_dist[size_src_i];
        int_t main_out_degree = o_dist->genOutDegree(cumu_row_id_i);
        int_t extra_out_degree = (rand.nextReal() < comm_rho) ? extraDegree(od_max - main_out_degree + 10, comm_rho + 1.0) : 0;
        std::vector<int_t> extra_out_degree_distr = extra_out_degree ? Utility::splitDegree(split, extra_out_degree, 1) : std::vector<int_t>();
        // if (extra_out_degree) std::cout << "i, extra_out_degree: " << i << ", " << extra_out_degree << std::endl;

        // i's communities (source)
        std::set<int_t> comms_i({ sp_row_i });
        for (auto& ol_anchor_comm : olAnchorComm[sp_row_i]) {
            double ol = ol_anchor_comm.second;
            int_t thre_row_i = (int_t)(size_src_i * ol);
            if (cumu_row_id_i > thre_row_i) comms_i.insert(ol_anchor_comm.first);
        }

        int_t cumu_col = 0;
        int_t sp_col_j = 0;
        std::set<std::pair<int_t, int_t>> nbrs;
        while (sp_col_j < an_comms) {
            // std::cout << "\tsp_col_j: " << sp_col_j << std::endl;
            int_t size_src_j = split[sp_col_j][0];
            int_t size_trg_j = split[sp_col_j][1];

             //int_t extra_out_degree_j = Utility::mathRound(extra_out_degree * 1.0 * size_trg_j / (1.0 * t_nodes));
            int_t extra_out_degree_j = extra_out_degree ? extra_out_degree_distr[sp_col_j] : 0;

            // within communities
            if (sp_col_j == sp_row_i) {
                // coincident parts: (a+b)->(a+b)
                Distribution *i_dist = col_dist[size_trg_j];
                for (int_t ti = 0; ti < main_out_degree; ) {
                    int_t t = i_dist->genTargetID();
                    while (is_homo && i == t + cumu_col) { t = i_dist->genTargetID(); }
                    // time window of community `sp_col_i`
                    //int_t ts = extra_timer.genTimestamp(windSplit[sp_row_i][0], windSplit[sp_row_i][1]);
                    int_t ts = main_timer[sp_row_i]->genTimestamp();
                    if (nbrs.insert({ t + cumu_col, ts }).second) ++ti;
                }
            } else {
                // overlapping parts
                bool b_overlap_ij = olAnchorComm[sp_row_i].find(sp_col_j) != olAnchorComm[sp_row_i].end();
                if (b_overlap_ij && sp_row_i < sp_col_j) {
                    double ol = olAnchorComm[sp_row_i][sp_col_j];
                    int_t thre_row_i = (int_t)(size_src_i * ol);
                    if (cumu_row_id_i + 1 >= thre_row_i) {
                        // TODO: (b+c)->(b+c): b->b
                        int_t ol_num = main_out_degree * (1.0 - ol) * (1.0 - ol);        // TODO
                        int_t sp_size = (int_t)(size_trg_i * ol);
                        int_t ol_size = size_trg_i - sp_size;
                        //std::cout << "\t\tb->b: ol_num: " << ol_num << ", sp_size: " << sp_size << ", ol_size: " << ol_size << std::endl;
                        for (int_t ti = 0; ti < ol_num; ) {
                            int_t t = rand.nextInt(ol_size - 1);
                            while (is_homo && i == t + cumu_col_psum[sp_row_i] + sp_size) { t = rand.nextInt(ol_size - 1); }
                            // time window of community `sp_col_j`
                            //int_t ts = extra_timer.genTimestamp(windSplit[sp_col_j][0], windSplit[sp_col_j][1]);
                            int_t ts = main_timer[sp_col_j]->genTimestamp();
                            //std::cout << "\t\t\twant to add (" << i << "," << t + cumu_col_psum[sp_row_i] + sp_size << ")" << std::endl;
                            if (nbrs.insert({ t + cumu_col_psum[sp_row_i] + sp_size, ts }).second) ++ti;
                        }

                        // (b+c)->(b+c): b->c
                        ol_num = main_out_degree * (1.0 - ol);       // TODO
                        for (int_t ti = 0; ti < ol_num; ) {
                            int_t t = rand.nextInt(size_trg_j - 1);
                            // time window of community `sp_col_j`
                            //int_t ts = extra_timer.genTimestamp(windSplit[sp_col_j][0], windSplit[sp_col_j][1]);
                            int_t ts = main_timer[sp_col_j]->genTimestamp();
                            if (nbrs.insert({ t + cumu_col, ts }).second) ++ti;
                        }
                    }
                    
                }
                if (b_overlap_ij && sp_row_i > sp_col_j) {
                    // (b+c)->(b+c): c->b
                    double ol = olAnchorComm[sp_row_i][sp_col_j];
                    int_t sp_size = (int_t)(size_trg_j * ol);
                    int_t ol_size = size_trg_j - sp_size;
                    int_t ol_num = main_out_degree * (1.0 - ol);   // TODO
                    for (int_t ti = 0; ti < ol_num; ) {
                        int_t t = rand.nextInt(ol_size - 1);
                        // time window of community `sp_col_j`
                        //int_t ts = extra_timer.genTimestamp(windSplit[sp_col_j][0], windSplit[sp_col_j][1]);
                        int_t ts = main_timer[sp_col_j]->genTimestamp();
                        if (nbrs.insert({ t + cumu_col + sp_size, ts }).second) ++ti;
                    }
                }
            }
            int_t within_edges = nbrs.size();

            // beyond anchor community or between anchor communities
            for (int_t ti = 0; ti < extra_out_degree_j; ) {
                int_t t = rand.nextInt(size_trg_j - 1);
                while (sp_row_i == sp_col_j && is_homo && i == t + cumu_col) { t = rand.nextInt(size_trg_j - 1); }
                // t's communities (target)
                std::set<int_t> comms_j({sp_col_j});
                for (auto& ol_anchor_comm : olAnchorComm[sp_col_j]) {
                    double ol = ol_anchor_comm.second;
                    int_t thre_col_j = (int_t)(size_trg_j * ol);
                    if (t > thre_col_j) comms_j.insert(ol_anchor_comm.first);
                }
                std::set<int_t> comms_ij;   // intersection set
                set_intersection(comms_i.begin(), comms_i.end(), comms_j.begin(), comms_j.end(), 
                    inserter(comms_ij, comms_ij.begin()));
                if (comms_ij.empty()) {
                    // beyond: assign any timestamp
                    int_t ts = extra_timer->genTimestamp();
                    //std::cout << "\t\tts(beyond): " << ts << std::endl;
                    if (nbrs.insert({t + cumu_col, ts}).second) ++ti;   // succeed
                } else {
                    // between: common time window
                    std::vector<std::vector<int_t>> window_ij;
                    for (auto comm : comms_ij) { window_ij.push_back(windSplit[comm]); }
                    auto window_ij_unn = Utility::unionWindow(window_ij);
                    auto window_ij_cpl = Utility::compleWindow(window_ij_unn,
                        temp_params[schema::json_temp_min_timestamp],
                        temp_params[schema::json_temp_max_timestamp]);
                    if (window_ij_cpl.empty()) continue;
                    //        : assign some timestamp that is not inside window_ij
                    int_t ts = extra_timer->genTimestamp(window_ij_cpl);
                    if (nbrs.insert({ t + cumu_col, ts }).second) ++ti;   // succeed                        
                }
            }

            // if (is_homo && nbrs.count(i)) {
            //     nbrs.erase(i);
            // }

            int_t all_edges = nbrs.size();

#ifdef PARALLEL
            #pragma omp atomic
#endif
            actual_edges += all_edges;
            
#ifdef PARALLEL
            #pragma omp atomic
#endif
            extra_edges += all_edges - within_edges;
            
            // PATCH, for Co-occurrence (Person - Page - Person => Person - Person)
#ifdef PATCH_VPP
            if (b_patch) {
                for (auto n : nbrs) {
                    int_t ri = rand.nextInt(patch_contact_list.size() - 1);
                    std::string attach = patch_contact_list[ri] + "\tvp_" + std::to_string(i) + "_VPerson\tvp_" + std::to_string(n) + "_VPerson\tContactRelation";
                    store_ptr->writeTSVLine(i, n, attach);
                }
            } else {
                store_ptr->writeLine(i, nbrs);
            }
#else
            store_ptr->writeLine(i, nbrs);
#endif
            // END PATCH

            nbrs.clear();
            cumu_col += size_trg_j;
            sp_col_j ++;
        }

        cur = (double)actual_edges / (double) n_edges;
        if (cur - progress >= 0.01) {
#ifdef PARALLEL
            #pragma omp atomic
#endif
            progress += 0.01;
            gp_progress = progress;
            //progress_bar.set_progress(progress);
        }
    }
    // generate END

    // Store close
#ifdef PARALLEL
    for (int i = 0; i < n_threads; ++i) {
        store_list[i]->close();
    }
#else
    store->close();
#endif

    gp_progress = 1.0;
    // Output info
    std::cout << "[Generation::temporalSocialGraph] #Source Nodes = " << s_nodes << " , #Target Nodes = " << t_nodes << std::endl;
    std::cout << "[Generation::temporalSocialGraph] #Actual Edges = " << actual_edges << std::endl;
    std::cout << "[Generation::temporalSocialGraph] #Extra Edges = " << extra_edges << std::endl;
    std::cout << "[Generation::temporalSocialGraph] #Expect Edges = " << n_edges << std::endl;
    actual_edge_nums[st_edge.e_label] = actual_edges;
}

void Generation::embeddedGraph(St_EdgeGeneration& st_edge) {

}

Distribution* Generation::getDist(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params,
        bool out_pp, std::string& d_type) {
    Distribution* ans = nullptr;
    if (d_type == schema::json_dist_PowerLaw) {
        ans = new PowerLaw(mid, mxd, n, m, params);
        ans->preProcess(out_pp);
    } else if (d_type == schema::json_dist_Normal) {
        ans = new Normal(mid, mxd, n, m, params);
        ans->preProcess(out_pp);
    } else if (d_type == schema::json_dist_LogNormal) {
        ans = new LogNormal(mid, mxd, n, m, params);
        ans->preProcess(out_pp);
    } else if (d_type == schema::json_dist_Uniform) {
        ans = new Uniform(mid, mxd, n, m, params);
    } else {
        std::cerr << "[Generation::getDist] Unknown distribution: " << d_type << std::endl;
    }
    return ans;
}

OutDegreeDistribution* Generation::getOutDist(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params, std::string& d_type) {
    OutDegreeDistribution* ans = nullptr;
    if (d_type == schema::json_dist_PowerLaw) {
        ans = new OutPowerLaw(mid, mxd, n, m, params);
        ans->pre_propcess();
    } else if (d_type == schema::json_dist_Normal) {
        ans = new OutNormal(mid, mxd, n, m, params);
        ans->pre_propcess();
    } else if (d_type == schema::json_dist_LogNormal) {
        ans = new OutLogNormal(mid, mxd, n, m, params);
        ans->pre_propcess();
    } else {
        std::cerr << "[Generation::getOutDist] Unknown distribution: " << d_type << std::endl;
    }
    return ans;
}

Timestamp* Generation::getTimer(int_t mit, int_t mat, std::unordered_map<std::string,
    double>& params, const std::string& t_type) {
    Timestamp* ans = nullptr;
    if (t_type == schema::json_temp_PowerLaw) {
        ans = new TimestampPowerLaw(mit, mat, params);
        ans->preProcess();
    } else if (t_type == schema::json_temp_Normal) {
        ans = new TimestampNormal(mit, mat, params);
        ans->preProcess();
    } else if (t_type == schema::json_temp_LogNormal) {
        ans = new TimestampLogNormal(mit, mat, params);
        ans->preProcess();
    } else if (t_type == schema::json_temp_Uniform) {
        ans = new TimestampUniform(mit, mat, params);
    } else { std::cerr << "[Generation::getTimer] Unknown distribution: " << t_type << std::endl; }
    return ans;
}

int_t Generation::extraDegree(int_t max_degree, double c) {
    double a = exp(-1.0 / c);
    double b = a - exp(-max_degree * 1.0 / c);
    double y = rand.nextReal();
    int_t ans = (int_t)(-c * log(a - y * b));
    if (ans < 0) {
        ans = 0;
    }
    return ans;
}

bool Generation::path_exists(std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info))
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

bool Generation::mkdir(std::string& path) {
    if (path_exists(path)) {

#ifdef DEBUG
        std::cout << "[Generation::mkdir] " << path << " already exits." << std::endl;
#endif

        return true;
    }
    
    std::string pathname;
    std::string md_cmd = "mkdir ";
#ifdef _WIN32
    for (char ch : path) {
        if (ch == '/')
            pathname.push_back('\\');
        else
            pathname.push_back(ch);
    }
    md_cmd += pathname;
#else
    md_cmd += " -p " + path;
#endif

    int res = system(md_cmd.c_str());

#ifdef DEBUG
    if (res == 0)
        std::cout << "[Generation::mkdir] Create " << path << " successfully." << std::endl;
    else
        std::cout << "[Generation::mkdir] Create " << path << " failed." << std::endl;
#endif

    return (0 == res);
}

bool Generation::my_rename(std::string& src_name, std::string& tgt_name) {
    // std::string cmd = "mv " + src_name + " " + tgt_name;
    // system(cmd.c_str());
    rename(src_name.c_str(), tgt_name.c_str());
    return true;
}

} //! namespace fastsgg
} //! namespace gl


// void Generation::socialGraph(St_EdgeGeneration& st_edge)
            // // overlapping
            // double ol = 0.9;
            // if (sp_col_j == sp_row_i) {
            //     Distribution *i_dist = col_dist[split[sp_col_j][1]];
            //     while (nbrs.size() < num) {
            //         int_t t = i_dist->genTargetID();
            //         nbrs.insert(t + cumu_col);
            //     }
            // } else {
            //     int size = split[sp_col_j][1];
            //     while (nbrs.size() < num) {
            //         int_t t = rand.nextInt(size);
            //         nbrs.insert(t + cumu_col);
            //     }
            //     // comm i and comm i + 1 overlapping
            //     if (sp_row_i + 1 == sp_col_j) {
            //         int thre_row_i = (int)(split[sp_row_i][0] * ol);
            //         if (cumu_row >= thre_row_i) {
            //             int ol_num = num * 8;
            //             for (int ti = 0; ti < ol_num; ++ ti) {
            //                 int_t t = rand.nextInt(size);
            //                 nbrs.insert(t + cumu_col);
            //             }
            //         }
            //     }
            //     if (sp_row_i == sp_col_j + 1) {
            //         int sp_size = (int)(size * ol);
            //         int ol_size = size - sp_size;
            //         int ol_num = num * 2;
            //         for (int ti = 0; ti < ol_num; ++ti) {
            //             int_t t = rand.nextInt(ol_size);
            //             nbrs.insert(t + cumu_col + sp_size);
            //         }
            //     }
            // }
