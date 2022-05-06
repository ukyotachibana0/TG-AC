/**
 * create time : 2020-10-14
 * Generation header
 */
#pragma once

#include "types.hpp"
#include "Distribution.hpp"
#include "OutDegreeDistribution.hpp"
#include "Timestamp.hpp"

namespace gl {
namespace fastsgg {

typedef struct _generate_embedded {
    bool b_homo;
    std::string basename;
    std::string type;
    int_t s_nodes;
    int_t t_nodes;
    int_t n_edges;

    std::string outd_type;
    std::unordered_map<std::string, double> out_params;
    std::string ind_type;
    std::unordered_map<std::string, double> in_params;

    std::string comm_type;
    std::unordered_map<std::string, double> comm_params;
    std::vector<std::vector<int_t>> commSplit;

    std::string temp_type;
    std::unordered_map<std::string, double> temp_params;

    std::vector<std::vector<int_t>> windSplit;
    std::vector<std::unordered_map<int_t, double>> olAnchorComm;

    std::vector<int_t> s_mapping;
    std::vector<int_t> t_mapping;
} St_EmbeddedGeneration;

typedef struct _generate_edge {
    std::string e_label;
    // node type
    std::string e_source;
    std::string e_target;
    // 
    std::unordered_map<std::string, double> out_params;
    std::unordered_map<std::string, double> in_params;
    std::string ind_type;
    std::string outd_type;
    int_t s_nodes;
    int_t t_nodes;
    int_t n_edges;
    std::string basename;
    // temporal
    bool b_temporal;
    std::string temp_type;
    std::unordered_map<std::string, double> temp_params;
    // community
    bool b_social;
    std::unordered_map<std::string, double> comm_params;
    std::vector<std::vector<int_t>> commSplit;
        // for anchor community
    std::vector<std::vector<int_t>> windSplit;
    std::vector<std::unordered_map<int_t, double>> olAnchorComm;
        // for embedded anchor community
    bool b_embedded;
    St_EmbeddedGeneration st_embd;
    // overlap
    bool b_overlap;
    double dv_overlap;
    std::unordered_map<int_t, std::unordered_set<int_t>> overlapComm;
} St_EdgeGeneration; //! Topology Generation

typedef struct _generate_node_attribute {
    // TODO
} St_NodeAttrGeneration; //! Node Attribute

class Generation
{
private:
    std::string json_filename;
    JSON::json json_obj;

    Random rand;
    
    bool b_streaming_style;

    double g_gr;
    std::string g_format;
    std::vector<St_EdgeGeneration> edge_gen_plan;           // Generation Plan
    std::vector<St_NodeAttrGeneration> node_attr_gen_plan;  // Generation Plan
    
    int n_threads;
    const int thread_chunk_size = 16;

    double gp_progress;     // Generation Progress
    std::string gp_tag;     // Generation Tag (Node/Edge-{name/source_name-target_name})
    bool gb_gen_done;       // is Global Generation Progress Done
    bool gb_start_gen;      // has started generating

    std::unordered_map<std::string, int_t> actual_edge_nums;

public:
    Generation();

    Generation(std::string& filename);
    
    ~Generation();

    bool check_json();

    void generate_plan();

    void run();

    std::string currentGenerationTag();

    double currentGenerationProgress();

    bool isGenerationDone();

    bool hasGenerationStart();

    int_t getActualEdges(std::string e_label);

    void simpleGraph(St_EdgeGeneration& st_edge);

    void socialGraph(St_EdgeGeneration& st_edge);

    void streamingSimpleGraph(St_EdgeGeneration& st_edge);

    void streamingSocialGraph(St_EdgeGeneration& st_edge);

    void temporalSimpleGraph(St_EdgeGeneration& st_edge);

    void temporalSocialGraph(St_EdgeGeneration& st_edge);   // anchor communities

    void embeddedGraph(St_EmbeddedGeneration & st_embd);    // embedded anchor communities

    Distribution* getDist(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params,
        bool out_pp, std::string& d_type);

    OutDegreeDistribution* getOutDist(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params, std::string& d_type);

    Timestamp* getTimer(int_t mit, int_t mat, std::unordered_map<std::string,
        double>& params, const std::string& t_type);

    int_t extraDegree(int_t max_degree, double c);

    bool path_exists(std::string& path);

    bool mkdir(std::string& path);

    bool my_rename(std::string& src_name, std::string& tgt_name);

private:
    static bool check_json_temp(JSON::json& temp, std::string info);

    static bool check_json_comm(JSON::json& comm, std::string info, bool required);

    static bool check_json_dist(JSON::json& dist, std::string info);
}; //! class Generation

} //! namespace fastsgg
} //! namespace gl