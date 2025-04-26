#include "kmeans.hpp"
#include "bbox.hpp"
#include "primitive.h"
#include "../visualization/BVH.h"
#include "construction/timer.hpp"

#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <chrono>  // 用于计时
#include <fstream> // 用于写入CSV文件

using namespace std;
#define maxLeafNum 4

static int UNIQUE_ID = 0;

Kmeans::Kmeans(size_t iterCount, size_t K, size_t P, vector<Primitive *> primitives)
// 迭代次数、聚类数、随机点数、集几何体
{
    m_iterations = iterCount;
    m_K = K;
    m_P = P;
    this->unique_id = UNIQUE_ID++;
    this->primitives = primitives;
    cluster = new Cluster[m_K];
    children = new Kmeans *[m_K];
    children_existence = std::vector<bool>(m_K, true);
    BoundingBox cur_world;
    for (size_t i = 0; i < primitives.size(); ++i)
    {
        cur_world.expand(primitives[i]->get_bbox());
    }
    world = cur_world;

    std::vector<BoundingBox> kCentroids = getRandCentroidsOnMesh(m_K, m_P);
    for (size_t i = 0; i < m_K; ++i)
    {
        cluster[i].representive = kCentroids[i];
    }
}

Kmeans::~Kmeans()
{
    delete[] cluster;
}

// 随机选择在模型上的点而不是空间内的点
vector<BoundingBox> Kmeans::getRandCentroidsOnMesh(int k, int p)
{
    vector<BoundingBox> kCentroids;
    size_t idx_primitive;

    // 第一个随机点
    srand((unsigned)time(NULL));
    idx_primitive = rand() % primitives.size();
    kCentroids.push_back(primitives[idx_primitive]->get_bbox());

    srand((unsigned)time(NULL));
    // 选取之后k-1个点
    for (int i = 1; i < k; ++i)
    {
        // 随机p个点
        std::vector<BoundingBox> tempP;
#pragma omp parallel for
        for (int j = 0; j < p; j++)
        {
            idx_primitive = rand() % primitives.size();
            tempP.push_back(primitives[idx_primitive]->get_bbox());
        }

        // 选取与之前算出的点距离最远的点作为下一个representive
        int index = 0;
        float maxDistance = -1.0f;
#pragma omp parallel for reduction(max : maxDistance)
        for (int k = 0; k < p; k++)
        {
            float local_maxDistance = -1.0f;
            for (int q = 0; q < kCentroids.size(); q++)
            {
                BoundingBox bb;
                bb.expand(tempP[k]);
                bb.expand(kCentroids[q]);
                float distance = glm::length(bb.extent);
                if (distance > local_maxDistance)
                {
                    local_maxDistance = distance;
                }
            }
            if (local_maxDistance > maxDistance)
            {
#pragma omp critical // critical避免数据竞争
                {
                    if (local_maxDistance > maxDistance)
                    {
                        maxDistance = local_maxDistance;
                        index = k;
                    }
                }
            }
        }
        kCentroids.push_back(tempP[index]);
    }
    return kCentroids;
}

float Kmeans::calDistance(BoundingBox b1, BoundingBox b2)
{
    double min_value = glm::length(b1.min - b2.min);
    double max_value = glm::length(b1.max - b2.max);
    double res = min_value + max_value;
    return res;
}

void Kmeans::registerCallback(std::function<void(const BoundingBox, const bool)> func)
{
    callback_func = func;
}

// Parallel BVH Construction using k-means Clustering
// https://meistdan.github.io/publications/kmeans/paper.pdf
void Kmeans::constructKaryTree(int depth)
{
    // 计时开始
    auto start_time = std::chrono::high_resolution_clock::now();

    // 构造本层结构
    this->run();

    // 计时结束
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    timer::write_k_means_time(this->unique_id, depth, elapsed_us.count());

    // 判定子节点是否是叶子节点
    for (size_t i = 0; i < m_K; i++)
    {
        // 叶子cluster 该cluster[i]对应的children为NULL
        if (cluster[i].indexOfPrimitives.size() < maxLeafNum * m_K)
        {
            children_existence[i] = false;
            if (callback_func)
            {
                callback_func(cluster[i].world, true);
            }
            continue;
        }
    }
    // 构造本层完成，通过callback通知visualization已经发生改变
    if (callback_func)
    {
        callback_func(this->world, false);
    }
    // 对本层中的每个cluster循环构造下层
    for (size_t i = 0; i < m_K; i++)
    {
        // 跳过叶子children
        if (!children_existence[i])
            continue;
        // 否则DFS
        vector<Primitive *> pTemp;
        for (size_t p = 0; p < cluster[i].indexOfPrimitives.size(); ++p)
        {
            pTemp.push_back(primitives[cluster[i].indexOfPrimitives[p]]);
        }
        children[i] = new Kmeans(m_iterations, m_K, m_P, pTemp);
        if (callback_func)
        {
            children[i]->registerCallback(callback_func);
        }
        children[i]->constructKaryTree(depth + 1);
    }
}

// refinement of K-means tree using agglomerative clustering
void Kmeans::buttom2Top()
{
    this->root = agglomerativeClustering();
    for (size_t i = 0; i < this->m_K; ++i)
    {
        if (children[i] != NULL)
            this->children[i]->buttom2Top();
    }
}

// Fast Agglomerative Clustering for Rendering
// https://www.graphics.cornell.edu/~bjw/IRT08Agglomerative.pdf
KBVHNode *Kmeans::agglomerativeClustering()
{
    vector<KBVHNode *> v;
    for (size_t i = 0; i < m_K; ++i)
    {
        BoundingBox bb;

        if (cluster[i].indexOfPrimitives.size() > 0)
        {
            for (size_t j = 0; j < cluster[i].indexOfPrimitives.size(); ++j)
                bb.expand(primitives[cluster[i].indexOfPrimitives[j]]->get_bbox());

            v.push_back(new KBVHNode(bb, cluster[i]));
        }
    }
    while (v.size() > 1)
    {
        // sah代价最小
        double minCost = numeric_limits<double>::max();
        size_t left, right;
        for (size_t i = 0; i < v.size(); ++i)
            for (size_t j = 0; j < v.size(); ++j)
            {
                if (i == j)
                    continue;
                // sa*na+sb*nb
                double cost = v[i]->c.indexOfPrimitives.size() * v[i]->bb.surface_area() + v[j]->c.indexOfPrimitives.size() * v[j]->bb.surface_area();
                if (cost < minCost)
                {
                    minCost = cost;
                    left = i;
                    right = j;
                }
            }
        if (left > right)
        {
            size_t temp = left;
            left = right;
            right = temp;
        }

        KBVHNode *newNode = combine(v[left], v[right]);
        v.erase(v.begin() + left);
        v.erase(v.begin() + right - 1);
        v.push_back(newNode);
    }
    return v[0];
}

KBVHNode *Kmeans::combine(KBVHNode *a, KBVHNode *b)
{
    Cluster c;
    BoundingBox bb;
    for (size_t i = 0; i < a->c.indexOfPrimitives.size(); ++i)
    {
        c.indexOfPrimitives.push_back(a->c.indexOfPrimitives[i]);
        bb.expand(primitives[a->c.indexOfPrimitives[i]]->get_bbox());
    }

    for (size_t i = 0; i < b->c.indexOfPrimitives.size(); ++i)
    {
        c.indexOfPrimitives.push_back(b->c.indexOfPrimitives[i]);
        bb.expand(primitives[b->c.indexOfPrimitives[i]]->get_bbox());
    }
    KBVHNode *res = new KBVHNode(bb, c);
    res->l = a;
    res->r = b;
    return res;
}

void Kmeans::run()
{
    // 预分配内存
#pragma omp parallel for
    for (size_t i = 0; i < m_K; ++i)
    {
        cluster[i].indexOfPrimitives.reserve(primitives.size() / m_K * 2);
    }

    for (size_t iter = 0; iter < m_iterations; ++iter)
    {
        // 更新representives cluster重新分配
#pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < m_K; ++i)
        {
            if (iter != 0)
            {
                cluster[i].updateRepresentive();
            }
            cluster[i].reset();
            cluster[i].indexOfPrimitives.clear();
        }

        // 计算距离并分配至最近的cluster
        std::vector<size_t> nearestCluster(primitives.size());

        // 并行计算每个primitive的最近cluster索引
#pragma omp parallel for
        for (size_t idx_primitives = 0; idx_primitives < primitives.size(); ++idx_primitives)
        {
            size_t index = 0;
            double minDistance = std::numeric_limits<double>::max();
            BoundingBox temp = primitives[idx_primitives]->get_bbox();
            for (size_t idx_clusters = 0; idx_clusters < m_K; ++idx_clusters)
            {
                double dist = calDistance(temp, cluster[idx_clusters].representive);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    index = idx_clusters;
                }
            }
            nearestCluster[idx_primitives] = index;
        }

        // 根据计算结果将primitive分配到对应cluster
        for (size_t idx_primitives = 0; idx_primitives < primitives.size(); ++idx_primitives)
        {
            size_t index = nearestCluster[idx_primitives];
            cluster[index].add(idx_primitives, primitives[idx_primitives]->get_bbox());
        }
    }
    // print();
}

// void Kmeans::traverse_cluster(std::vector<float> *vertices, std::vector<float> *colors, std::vector<int> *indices) const {
//     glm::vec3 min = world.min;
//     glm::vec3 max = world.max;
//     glm::vec3 color(1);
//     bool is_leaf = true;
//     for (size_t child_i = 0; child_i < this->m_K; ++child_i) {
//         if (children_existence[child_i]) {
//             is_leaf = false;
//             break;
//         }
//     }
//     if(is_leaf) {
//         color = glm::vec3(0,1,0);
//     }
//     BVH::buildCube(vertices, colors, indices, min, max, color);
// }

void Kmeans::print() const
{
    cout << "**********************************" << endl;
    for (size_t i = 0; i < m_K; ++i)
    {
        cout << "#" << i << " Cluster: " << cluster[i].indexOfPrimitives.size() << endl;
    }
    cout << "**********************************" << endl;
    cout << endl;
}
