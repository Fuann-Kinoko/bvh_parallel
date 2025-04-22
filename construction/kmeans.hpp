#include "bbox.hpp"
#include "cluster.hpp"
#include "primitive.h"
#include <functional>

struct KBVHNode {
public:
    BoundingBox bb;
    Cluster c;
    KBVHNode* l;
    KBVHNode* r;

    inline bool isLeaf() const { return l == NULL && r == NULL; }

    KBVHNode(BoundingBox bb, Cluster c)
        : bb(bb)
        , c(c)
        , l(NULL)
        , r(NULL)
    {
    }
};

class Kmeans {
public:
    Kmeans()
    {
        cluster = NULL;
        children = NULL;
    }

    Kmeans(size_t iterCount, size_t K, size_t P, std::vector<Primitive *> primitives);
    ~Kmeans();

    // 执行
    void run();

    // 注册与渲染沟通的callback
    void registerCallback(std::function<void (const BoundingBox, const bool)> func);

    // 从上至下构造k叉树
    void constructKaryTree(int depth);

    // 从下至上 凝聚聚类(AC)构造二叉树
    KBVHNode* agglomerativeClustering();

    // 自底向上递归构造
    void buttom2Top();

    // 打印结果
    void print() const;

    // 传递当前的cluster内容到外部数组中, 但不修改当前内容
    void traverse_cluster(std::vector<float> *vertices, std::vector<float> *colors, std::vector<int> *indices) const;

    // 迭代次数
    size_t m_iterations;
    // 参数K
    size_t m_K;
    // 参数P
    size_t m_P;

    int unique_id;

    // 聚类体
    Cluster* cluster;

    // k_叉树结构
    Kmeans** children;

    BoundingBox world;

    // 本层构造出的二叉树根节点（只有用agglomerative算法时会用到）
    KBVHNode* root;

    // 输入数据
    std::vector<Primitive *> primitives;

private:
    // 计时用
    // Timer timer;

    std::vector<bool> children_existence;
    // 与渲染进行沟通的callback
    std::function<void (const BoundingBox, const bool)> callback_func;

    // 初始化随机点
    std::vector<BoundingBox> getRandCentroidsOnMesh(int k, int p);
    // 距离公式
    float calDistance(BoundingBox b1, BoundingBox b2);
    // 合并两个KBVHNode (agglomerativeClustering用)
    KBVHNode* combine(KBVHNode* a, KBVHNode* b);
};