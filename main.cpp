/**
 * create time : 2020-11-02
 * Main procedure
 */

#include "Generation.hpp"
#include "gen_attribute.hpp"
#include <chrono>

using namespace std;
using namespace gl::fastsgg;

class TimeCounter {
private:
    std::chrono::high_resolution_clock::time_point tp_start;
    std::chrono::high_resolution_clock::time_point tp_stop;

public:
    void start() {
        tp_start = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        tp_stop = std::chrono::high_resolution_clock::now();
    }

    double milliseconds() {
        auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(tp_stop - tp_start);
        return ts.count();
    }

    double seconds() {
        auto ts = std::chrono::duration_cast<std::chrono::seconds>(tp_stop - tp_start);
        return ts.count();
    }
};

// extern bool gen_main(string attr_json, string comm_fn, string output_fn);

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    TimeCounter tc;
    tc.start();
    const char* filename = argv[1];
    string str_filename(filename);
    Generation gen(str_filename);
    gen.run();
    tc.stop();
    cout << "[Main] Elapsed time : " << tc.milliseconds() << " ms." << endl;

    /*const char* ch_attr_json = argv[1];
    const char* ch_comm_fn = argv[2];
    const char* ch_output_fn = argv[3];
    const char* ch_edge_fn = argv[4];
    string str_attr_json(ch_attr_json);
    string str_comm_fn(ch_comm_fn);
    string str_output_fn(ch_output_fn);
    string str_edge_fn(ch_edge_fn);
    bool res = gen_main(str_attr_json, str_comm_fn, str_output_fn, str_edge_fn);
    cout << "Done: " << res << endl;*/

    return 0;
}

// Overlapping Communities Generation Formally
// 
// 
// Node Attributes Generation
// 0. 数据类型：类目型、连续数值型
// 1. 抽象出一层，类目型数据，连续数值型也转换为类目型
// 2. 类目型进一步分为：many-value, multiple-value 类型
// 3. 分别为：
// 4. many-value: 每个属性有多个值，但，只能选择一个值
// 5. multiple-value: 每个属性有多个值，可以选择多个值
// 6. 分为多个 key-value 形式
// 7. 对于 many-value: 
// 8. key -> 对应多候选值，数字化，从 `1` 开始
// 9. 对于 multiple-value:
// 10. key_0 -> value_0
// 11. key_1 -> value_1
// 12. key_2 -> value_2
// 13. ...
// 14. 其中，value_0, value_1, ...
// 15. 值为 0 或 1
// 16. 0: 表示不选择这个值
// 17. 1: 表示选择这个值
// 18. 对于同一类型节点之间有边相连的情况
// 19. 以社区划分为准，进行属性的分配，保证一定的相似性
// 20. 对于同一类型节点之间没有边相连，但与其他类型的边相连
// 21. 需要重划分，然后确定社区归属，再生成
// 22. 属性相似性：优先保证条件分布，然后考虑社区相似性，最后考虑拓扑结构相似性（？）


// 1. 指定法，自顶向下方法
// 对于每个社区，构建一个虚拟中心节点，确定其属性表示，社区中其他节点均要保证与其一定的相似性
// 1) 生成 N_comm 个社区中心虚拟节点的属性，仅 K 个有值即可，K 个值不应该包括涉及到条件分布的属性
// 2) 对于每个社区，生成社区中节点的属性，首先，与所在社区的虚拟中心节点属性相同，然后，其他属性随机生成，保证条件分布
// 3) 最后，根据 社区间边的拓扑关系 保证一定程度的相似性，低于 社区内部的属性相似性。