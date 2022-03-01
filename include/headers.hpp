/**
 * create time : 2020-10-08
 * Header files for FastSGG
 */
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
// #include <random>
#include <cmath>    // isnan
#include <ctime>
#include "json.hpp"
#include "Random.hpp"
#include "Utility.hpp"
#include "types.hpp"

// #define DEBUG
#undef DEBUG

#define PARALLEL
// #undef PARALLEL

// #define PATCH_VPP
#undef PATCH_VPP

namespace JSON = nlohmann;

namespace gl {
namespace fastsgg {

enum class EnumStoreFormat : char {
    TSV,
    ADJ,
    CSR
};

namespace schema {

const static std::string json_graph = "graph";
// node schema
const static std::string json_node = "node";
const static std::string json_node_label = "label";
const static std::string json_node_amount = "amount";
// edge schema
const static std::string json_edge = "edge";
const static std::string json_edge_label = "label";
const static std::string json_edge_source = "source";
const static std::string json_edge_target = "target";
const static std::string json_edge_amount = "amount";
// distribution
const static std::string json_in_dist = "in";
const static std::string json_out_dist = "out";
const static std::string json_dist_type = "type";
const static std::string json_dist_PowerLaw = "power_law";
const static std::string json_dist_Normal = "normal";   // Gaussian
const static std::string json_dist_Uniform = "uniform";
const static std::string json_dist_LogNormal = "log_normal";
const static std::string json_dist_min_degree = "min_d";
const static std::string json_dist_max_degree = "max_d";
const static std::string json_dist_PL_lambda = "lambda";
const static std::string json_dist_Nor_mu = "mu";
const static std::string json_dist_Nor_sigma = "sigma";
// community
const static std::string json_comm = "community";
const static std::string json_comm_amount = "amount";
const static std::string json_comm_rho = "rho";
const static std::string json_comm_lambda = "lambda";
const static std::string json_comm_overlap = "overlap";
// streaming
const static std::string json_gr = "gr";
// storage format
const static std::string json_store_format = "store_format";
const static std::string json_format_ADJ = "ADJ";
const static std::string json_format_TSV = "TSV";   // Default format
const static std::string json_format_CSR = "CSR";

} //! namespace schema

// PATCH
const static std::vector<std::string> patch_contact_list({
    // "同航班", "同车次", "同住", "会面", "通话", "亲属", "好友"
    "a", "b", "c"
});
// END PATCH

} //! namespace fastsgg
} //! namespace gl
