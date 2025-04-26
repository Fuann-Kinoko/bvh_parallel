#ifndef CLUSTER_H_
#define CLUSTER_H_

#include "bbox.hpp"
#include <vector>
#include <omp.h>

class Cluster
{
public:
    BoundingBox world;
    BoundingBox representive;
    std::vector<size_t> indexOfPrimitives;

    Cluster() : m_min(0.0f), m_max(0.0f)
    {
        omp_init_lock(&lock);
        indexOfPrimitives.reserve(256); // 初始预分配
    }

    ~Cluster()
    {
        omp_destroy_lock(&lock);
    }

    // 线程安全的内存预分配
    void reserve(size_t capacity)
    {
        omp_set_lock(&lock);
        indexOfPrimitives.reserve(capacity);
        omp_unset_lock(&lock);
    }

    // 更新中心点
    void updateRepresentive()
    {
        omp_set_lock(&lock);
        if (!indexOfPrimitives.empty())
        {
            const float inv_size = 1.0f / indexOfPrimitives.size();
            representive = BoundingBox(m_min * inv_size, m_max * inv_size);
        }
        omp_unset_lock(&lock);
    }

    void reset()
    {
        omp_set_lock(&lock);
        indexOfPrimitives.clear();
        m_min = glm::vec3(0.0f);
        m_max = glm::vec3(0.0f);
        world = BoundingBox();
        // 这里的乘法是为了避免在多线程中出现数据竞争
        // 乘以0可以避免数据竞争
        m_min *= 0;
        m_max *= 0;
        omp_unset_lock(&lock);
    }

    void add(size_t i, BoundingBox bb)
    {
        omp_set_lock(&lock);
        // // 动态扩容策略：容量不足时倍增
        // if (indexOfPrimitives.size() >= indexOfPrimitives.capacity()) {
        //     indexOfPrimitives.reserve(indexOfPrimitives.capacity() * 2);
        // }
        indexOfPrimitives.push_back(i);
        m_min += bb.min;
        m_max += bb.max;
        world.expand(bb);
        omp_unset_lock(&lock);
    }

    glm::vec3 get_min() { return m_min; }
    glm::vec3 get_max() { return m_max; }

private:
    glm::vec3 m_min;
    glm::vec3 m_max;
    omp_lock_t lock;
};
#endif // CLUSTER_H_