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

    glm::vec3 m_min;
    glm::vec3 m_max;

    // 更新中心点
    void updateRepresentive()
    {
        const float inv_size = 1.0f / indexOfPrimitives.size();
        representive = BoundingBox(m_min * inv_size, m_max * inv_size);
    }

    void reset()
    {
        m_min *= 0;
        m_max *= 0;
    }

    void add(size_t i, BoundingBox bb)
    {
        indexOfPrimitives.push_back(i);
        m_min += bb.min;
        m_max += bb.max;
        world.expand(bb);
    }
};
#endif // CLUSTER_H_