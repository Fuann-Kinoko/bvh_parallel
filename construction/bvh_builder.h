#ifndef BVH_BUILDER_H_
#define BVH_BUILDER_H_

#include "bbox.hpp"
#include "primitive.h"
#include "kmeans.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "OBJ_Loader.h"

class BVHBuilder {
public:
    std::string import_path = "";

    static std::shared_ptr<BVHBuilder> LoadFromObj(const std::string& path);
    const std::vector<Primitive>& GetPrimitives() const { return pri; }
    void SetCallback(std::function<void(const BoundingBox, const bool)> callback) { m_callback = callback; }
    void Build();
private:
    std::vector<Primitive> pri;
    std::function<void(const BoundingBox, const bool)> m_callback;
};
#endif // BVH_BUILDER_H_