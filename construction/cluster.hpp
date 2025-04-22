#ifndef CLUSTER_H_
#define CLUSTER_H_

#include "bbox.hpp"
#include <vector>

class Cluster {
public:
    BoundingBox world;
    BoundingBox representive;
    std::vector<size_t> indexOfPrimitives;

    // 更新中心点
    void updateRepresentive()
    {
        representive = BoundingBox(m_min /= indexOfPrimitives.size(), m_max /= indexOfPrimitives.size());
        // cout<<representive<<endl;
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

    glm::vec3 get_min() {return m_min;}
    glm::vec3 get_max() {return m_max;}

private:
    glm::vec3 m_min;
    glm::vec3 m_max;
};
#endif // CLUSTER_H_