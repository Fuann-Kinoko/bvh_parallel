#ifndef VERTEX_H_
#define VERTEX_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

class Vertex {
public:
    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;

    Vertex(const vec3& pos, const vec3& norm, const vec2& tex)
        : Position(pos), Normal(norm), TexCoords(tex) {}
};


#endif // VERTEX_H_
