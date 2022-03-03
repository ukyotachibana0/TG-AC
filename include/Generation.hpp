/**
 * create time : 2020-10-14
 * Generation header
 */
#pragma once

#include "types.hpp"
#include "Distribution.hpp"
#include "OutDegreeDistribution.hpp"

namespace gl {
namespace fastsgg {

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
    std::string filename;
    std::string basename;
    // community
    bool b_social;
    std::unordered_map<std::string, double> comm_params;
    std::vector<std::vector<int_t>> commSplit;
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
    bool b_temporal;
    bool b_temporal_anchor;

    double g_gr;
    std::string g_format;
    EnumStoreFormat g_enum_format;
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

    Distribution* getDist(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params,
        bool out_pp, std::string& d_type);

    OutDegreeDistribution* getOutDist(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params, std::string& d_type);

    int_t extraDegree(int_t max_degree, double c);

    bool path_exists(std::string& path);

    bool mkdir(std::string& path);

    bool my_rename(std::string& src_name, std::string& tgt_name);

}; //! class Generation

} //! namespace fastsgg
} //! namespace gl