#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include "bbox.hpp"
#include "vertex.h"

class Primitive {
public:
    Vertex vertices[3];

    Primitive(const Vertex &v1, const Vertex &v2, const Vertex &v3) : vertices{v1, v2, v3} { }

    BoundingBox get_bbox() {
        if (bbox_initialized_) {
            return bbox;
        }

        BoundingBox bbox_new(vertices[0].Position);
        bbox_new.expand(vertices[1].Position);
        bbox_new.expand(vertices[2].Position);

        bbox_initialized_ = true;
        bbox = bbox_new;
        return bbox;
    }
private:
    BoundingBox bbox;
    bool bbox_initialized_ = false;
};
#endif // PRIMITIVE_H_