#pragma once

#include "construction/bbox.hpp"
#include "glad/glad.h"
// #include <GL/glew.h>

#include "../renderengine/camera/ACamera.h"
#include "../renderengine/shader/ShaderProgram.h"

#include "BVH.h"
#include "../construction/bvh_builder.h"
#include <mutex>
#include <thread>

class BVHVisualizationRenderer {
public:
    void init(const std::string &path);
    void init_construction(const std::string &path);

    void render(ACamera *camera, int windowWidth, int windowHeight);

    void cleanUp(bool full = false);

    void initSideVisualization();
    void updateSideVisualization();
    void updateBVHVisualization();
    void blockUntilBuildComplete();

    int m_bvhVisualizationMinLevel = 0;
    int m_bvhVisualizationMaxLevel = 32;
    bool m_leafGreen = true;

    glm::vec3 m_position{0};
    glm::vec3 m_rotation{0};
    glm::vec3 m_scale{1};

private:
    std::mutex side_data_mutex;
    std::mutex task_mutex;
    std::vector<float> side_vertices;
    std::vector<float> side_colors;
    std::vector<int> side_indices;
    int previous_side_size;
    int previous_side_indices_size;
    int previous_task_size;

    GLuint m_vaoId = 0;
    GLuint m_vertexVboId = 0;
    GLuint m_colorVboId = 0;
    GLuint m_indicesVboId = 0;

    GLsizei m_count = 0;

    ShaderProgram m_shaderProgram;
    std::shared_ptr<BVHBuilder> m_bvh_builder;
    std::vector<std::thread> m_bvh_builder_threads;
    std::vector<std::pair<BoundingBox, bool>> m_tasks;

    void update_bbox_under_construction(const BoundingBox world, const bool is_leaf);
};