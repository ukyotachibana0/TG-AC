/**
 * create time : 2020-10-09
 * Distribution hpp
 */
#pragma once

#include "headers.hpp"

namespace gl {
namespace fastsgg {

class Distribution
{
private:
    int_t min_degree;
    int_t max_degree;
    int_t degree_range;
    int_t num_nodes;
    int_t num_edges;

    // arrays and variables for getting out-degree simply
    int_t od_simple_cur_no;   // node ID
    int_t od_simple_cur_id;   // array index
    int_t od_simple_offset;   // offset
    int_t od_simple_mem_no;   // memory
    int_t od_simple_mem_id;   // 
    int_t od_simple_mem_off;  //
    std::vector<int_t> od_simple_cnum;
    // arrays and variables for getting out-degree intricately
    int bucket; // generate out-degree randomly
    double o_min_gap;
    double bucket_step; // [0,1]
    std::vector<int_t> od_complex_degree;

    // arrays and variables for getting target vertex ID
    std::vector<int_t> id_hash_tid; // corresponding target vertex ID
    std::vector<double> id_hash_cdf; // cumulative distribution function for sum of degrees
    std::vector<double> id_hash_ratio;   // corresponding ratio, for getting a ID in [id_hash_tid[i], id_hash_tid[i+1]]
    double i_min_gap;
    double i_min_rv;

public:
    // parameters of a distribution
    std::unordered_map<std::string, double> theta;

    // random number generator
    Random rand;

    Distribution();

    Distribution(int_t mid, int_t mxd, int_t n, int_t m,
        std::unordered_map<std::string, double>& params);

    ~Distribution();

    // build auxiliary function
    void preProcess(bool out);

    // set start source number
    void setStartSourceNo(int_t no);

    void setOutBucket(int x);

    // override
    virtual double pdf(int_t x) = 0;

    int_t numberOfMinDegree();

    int_t numberOfMaxDegree();

    int_t currentEdges();

    void matchEdges();

    void buildForOdSimply();

    void buildForOdIntricately();

    void buildForIdHashTables();

    int_t oHF(double x);

    int_t iHF(double x);

    int_t genOutDegreeSimple(int_t id);

    int_t genOutDegreeComplex(int_t id);

    virtual int_t genOutDegree(int_t id);

    virtual int_t genTargetID();

// ==================== For Debug ====================

    double getOMinGap();

    double getIMinGap();

    void printICdf();

    void printOCdf();

    // for multiple threads, deprecated
    std::vector<int> splitSourceNodes(int n_threads);

    std::vector<int_t> getOdNum();

}; //! class Distribution

} //! namespace fastsgg
} //! namespace gl