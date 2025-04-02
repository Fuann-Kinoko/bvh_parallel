#ifndef BBOX_H_
#define BBOX_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define INF_D (std::numeric_limits<double>::infinity())
#define INF_F (std::numeric_limits<float>::infinity())

class BoundingBox {
public:
    glm::vec3 max;
    glm::vec3 min;
    glm::vec3 extent;

    // default constructor to INF
    BoundingBox()
    {
        max = glm::vec3(-INF_F, -INF_F, -INF_F);
        min = glm::vec3(INF_F, INF_F, INF_F);
        extent = max - min;
    }

    // constructor for single point
    BoundingBox(const glm::vec3& p)
        : max(p)
        , min(p)
    {
        extent = max - min;
    }

    // constructor given two points
    BoundingBox(const glm::vec3& min, const glm::vec3& max)
        : max(max)
        , min(min)
    {
        extent = max - min;
    }

    // constructor given two points (xyz coordinates given acoordingly)
    BoundingBox(const double minX, const double minY, const double minZ,
        const double maxX, const double maxY, const double maxZ)
    {
        max = glm::vec3(maxX, maxY, maxZ);
        min = glm::vec3(minX, minY, minZ);
        extent = max - min;
    }

    // refer to another bounding box, expand its limit
    void expand(const BoundingBox& bbox)
    {
        min.x = fmin(min.x, bbox.min.x);
        min.y = fmin(min.y, bbox.min.y);
        min.z = fmin(min.z, bbox.min.z);
        max.x = fmax(max.x, bbox.max.x);
        max.y = fmax(max.y, bbox.max.y);
        max.z = fmax(max.z, bbox.max.z);
        extent.x = max.x - min.x;
        extent.y = max.y - min.y;
        extent.z = max.z - min.z;
    }

    // Expand the bounding box to include a new point in space.
    void expand(const glm::vec3& p)
    {
        min.x = std::fmin(min.x, p.x);
        min.y = std::fmin(min.y, p.y);
        min.z = std::fmin(min.z, p.z);
        max.x = std::fmax(max.x, p.x);
        max.y = std::fmax(max.y, p.y);
        max.z = std::fmax(max.z, p.z);
        extent = max - min;
    }

    // return the centroid point of the bounding box
    glm::vec3 centroid() const
    {
        glm::vec3 temp = min + max;
        temp.x /= 2.f;
        temp.y /= 2.f;
        temp.z /= 2.f;
        return temp;
    }

    // return the surface area of the bounding box
    float surface_area() const
    {
        if (empty())
            return 0.0;
        return 2 * (extent.x * extent.z + extent.x * extent.y + extent.y * extent.z);
    }

    /**
     * Compute the maximum dimension of the bounding box (x, y, or z).
     * return 0 if max dimension is x,
     *        1 if max dimension is y,
     *        2 if max dimension is z
     * TODO: replace with enum (or #define)
     *  - sure but please make sure indexing with the returned value still works
     *
     */
    uint8_t max_dimension() const
    {
        uint8_t d = 0;
        if (extent.y > extent.x)
            d = 1;
        if (extent.z > extent.y)
            d = 2;
        return d;
    }

    /**
     * Check if bounding box is empty.
     * Bounding box that has no size is considered empty. Note that since
     * bounding box are used for objects with positive volumes, a bounding
     * box of zero size (empty, or contains a single vertex) are considered
     * empty.
     */
    bool empty() const
    {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }

    /**
     * Calculate and return an object's
     * normalized position in the unit
     * cube defined by this BBox. if the
     * object is not inside of the BBox, its
     * position will be clamped into the BBox.
     *
     * \param pos the position to be evaluated
     * \return the normalized position in the unit
     * cube, with x,y,z ranging from [0,1]
     */
    glm::vec3 getUnitcubePosOf(glm::vec3 pos)
    {
        glm::vec3 o2pos = pos - min;
        if (extent.x == 0 && extent.y == 0 && extent.z == 0) {
            return glm::vec3();
        } else {
            glm::vec3 normalized_pos = o2pos / extent;
            return normalized_pos;
        }
    }
};
#endif // BBOX_H_