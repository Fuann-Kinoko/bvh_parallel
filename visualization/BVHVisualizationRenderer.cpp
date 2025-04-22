#include "BVHVisualizationRenderer.h"

#include "../renderengine/utils/Transformation.h"
#include "BVHVisualizationRenderLogic.h"
#include "construction/bbox.hpp"
#include <chrono>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

void BVHVisualizationRenderer::init(const std::string &path) {
    initSideVisualization();
    if(m_bvh_builder == NULL || m_bvh_builder->import_path != path) {
        cleanUp(true);
        initSideVisualization();
        m_bvh_builder = BVHBuilder::LoadFromObj(path);
        m_bvh_builder->SetCallback([this](const BoundingBox world, const bool is_leaf) {
            this->update_bbox_under_construction(world, is_leaf);
        });
        std::cout << "[WARNING] Object path changed to" << path << ", re-importing object" << std::endl;
    }
    // build BVH in a separate thread
    std::cout << "[Log] Start Building BVH in a separate thread" << std::endl;
    m_bvh_builder_threads.emplace_back(std::thread([this](){
        m_bvh_builder->Build();
    }));
}

void BVHVisualizationRenderer::blockUntilBuildComplete() {
    if(m_bvh_builder_threads.empty()) return;
    for(auto &t : m_bvh_builder_threads) {
        t.join();
    }
    std::cout << "[Log] Block Until Build Thread Completed" << std::endl;
    m_bvh_builder_threads.clear();
}

void BVHVisualizationRenderer::update_bbox_under_construction(const BoundingBox world, const bool is_leaf) {
    glm::vec3 min = world.min;
    glm::vec3 max = world.max;
    glm::vec3 color(1);
    if(is_leaf) {
        color = glm::vec3(0,1,0);
    }
    // kmeans_node->print();
    std::lock_guard<std::mutex> lock(side_data_mutex);
    BVH::buildCube(&side_vertices, &side_colors, &side_indices, min, max, color);
    // kmeans_node->traverse_cluster(&side_vertices, &side_colors, &side_indices);

    // // FIXME: debug用，用于展示过程，关掉可以显著减少时间
    // std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // std::cout << "LOG::current bounding box number : " << side_vertices.size() << std::endl;
    return;
}

void BVHVisualizationRenderer::render(ACamera *camera, int windowWidth, int windowHeight) {
    if(previous_side_size != side_vertices.size())
        updateSideVisualization();
    glm::mat4 modelMatrix = Transformation::getModelMatrix(m_position, m_rotation, m_scale);

    static const float FOV = glm::radians(90.0f);
    static const float Z_NEAR = 0.01f;
    static const float Z_FAR = 1000.f;
    glm::mat4 projectionMatrix = Transformation::getProjectionMatrix(FOV, static_cast<float>(windowWidth), static_cast<float>(windowHeight), Z_NEAR, Z_FAR);
    glm::mat4 viewMatrix = Transformation::getViewMatrix(camera);
    glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

    m_shaderProgram.bind();

    m_shaderProgram.setUniform("MVP", MVP);

    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_LINES, m_count, GL_UNSIGNED_INT, nullptr);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);

    ShaderProgram::unbind();
}

// default : full = false
void BVHVisualizationRenderer::cleanUp(bool full) {
    std::lock_guard<std::mutex> lock(side_data_mutex);
    previous_side_indices_size = 0;
    previous_side_size = 0;

    side_vertices.clear();
    side_colors.clear();
    side_indices.clear();

    if(m_vertexVboId)
        glDeleteBuffers(1, &m_vertexVboId);
    if(m_colorVboId)
        glDeleteBuffers(1, &m_colorVboId);
    if(m_indicesVboId)
        glDeleteBuffers(1, &m_indicesVboId);
    if(m_vaoId)
        glDeleteVertexArrays(1, &m_vaoId);

    m_shaderProgram.cleanUp();
    if(full)
        m_bvh_builder.reset();
    // FIXME: cleanup thread就算没有执行完也要强制退出
    blockUntilBuildComplete();
}

void BVHVisualizationRenderer::initSideVisualization() {
    cleanUp();

    std::lock_guard<std::mutex> lock(side_data_mutex);
    m_shaderProgram.init();
    m_shaderProgram.createVertexShader("./resources/shaders/bvh_visualization_vertex.glsl");
    m_shaderProgram.createFragmentShader("./resources/shaders/bvh_visualization_fragment.glsl");
    m_shaderProgram.link();
    m_shaderProgram.createUniform("MVP");
    ShaderProgram::unbind();

    m_count = side_indices.size();

    glGenVertexArrays(1, &m_vaoId);
    glBindVertexArray(m_vaoId);

    glGenBuffers(1, &m_vertexVboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVboId);
    glBufferData(GL_ARRAY_BUFFER, side_vertices.size() * sizeof(float), side_vertices.data(), GL_DYNAMIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, 10000000 * sizeof(float), side_vertices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &m_colorVboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_colorVboId);
    glBufferData(GL_ARRAY_BUFFER, side_colors.size() * sizeof(float), side_colors.data(), GL_DYNAMIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, 10000000 * sizeof(float), side_colors.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &m_indicesVboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, side_indices.size() * sizeof(int), side_indices.data(), GL_DYNAMIC_DRAW);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, 10000000 * sizeof(int), side_indices.data(), GL_DYNAMIC_DRAW);

    previous_side_size = side_vertices.size();
    previous_side_indices_size = side_indices.size();
    glBindVertexArray(0);
}

void BVHVisualizationRenderer::updateSideVisualization() {
    std::lock_guard<std::mutex> lock(side_data_mutex);
    m_count = side_indices.size();

    m_shaderProgram.bind();

    int current_side_size = side_vertices.size();
    int current_side_indices_size = side_indices.size();

    glBindVertexArray(m_vaoId);

    // 更新顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVboId);
    glBufferData(GL_ARRAY_BUFFER, side_vertices.size() * sizeof(float), side_vertices.data(), GL_DYNAMIC_DRAW);
    // glBufferSubData(GL_ARRAY_BUFFER,
    //     previous_side_size * sizeof(float),                         // start offset
    //     (current_side_size - previous_side_size) * sizeof(float),   // size
    //     side_vertices.data() + previous_side_size                   // start data pointer
    // );

    // 更新颜色数据
    glBindBuffer(GL_ARRAY_BUFFER, m_colorVboId);
    glBufferData(GL_ARRAY_BUFFER, side_colors.size() * sizeof(float), side_colors.data(), GL_DYNAMIC_DRAW);
    // glBufferSubData(GL_ARRAY_BUFFER,
    //     previous_side_size * sizeof(float),
    //     (current_side_size - previous_side_size) * sizeof(float),
    //     side_colors.data() + previous_side_size
    // );

    // 更新索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, side_indices.size() * sizeof(int), side_indices.data(), GL_DYNAMIC_DRAW);
    // glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
    //     previous_side_indices_size * sizeof(int),
    //     (current_side_indices_size - previous_side_indices_size) * sizeof(int),
    //     side_indices.data() + previous_side_indices_size
    // );

    previous_side_size = current_side_size;
    previous_side_indices_size = current_side_indices_size;
}

// void BVHVisualizationRenderer::updateBVHVisualization() {
//     // cleanUp();

//     bool init = !m_shaderProgram.is_valid();

//     if(init) {
//         m_shaderProgram.init();
//         m_shaderProgram.createVertexShader("./resources/shaders/bvh_visualization_vertex.glsl");
//         m_shaderProgram.createFragmentShader("./resources/shaders/bvh_visualization_fragment.glsl");
//         m_shaderProgram.link();
//         m_shaderProgram.createUniform("MVP");
//         ShaderProgram::unbind();
//     }
//     std::vector<float> vertices;
//     std::vector<float> colors;
//     std::vector<int> indices;

//     m_bvh.traverseBVH(&vertices, &colors, &indices, 0, 0, m_bvhVisualizationMinLevel, m_bvhVisualizationMaxLevel, m_leafGreen);

//     m_count = indices.size();

//     if(init) {
//         glGenVertexArrays(1, &m_vaoId);
//     }
//     glBindVertexArray(m_vaoId);

//     if(init) {
//         glGenBuffers(1, &m_vertexVboId);
//     }
//     glBindBuffer(GL_ARRAY_BUFFER, m_vertexVboId);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
//     if(init) {
//         glEnableVertexAttribArray(0);
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//     }

//     if(init) {
//         glGenBuffers(1, &m_colorVboId);
//     }
//     glBindBuffer(GL_ARRAY_BUFFER, m_colorVboId);
//     glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
//     if(init) {
//         glEnableVertexAttribArray(1);
//         glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
//     }

//     if(init) {
//         glGenBuffers(1, &m_indicesVboId);
//     }
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVboId);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

//     glBindVertexArray(0);
// }
