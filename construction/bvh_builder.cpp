#include "bvh_builder.h"
#include "construction/timer.hpp"
#include <mpi.h> // MPI

std::shared_ptr<BVHBuilder> BVHBuilder::LoadFromObj(const std::string& path) {
    // 创建 BVHBuilder 实例
    auto builder = std::make_shared<BVHBuilder>();
    builder->import_path = path;

    objl::Loader loader;
    bool loadSuccess = loader.LoadFile(path);

    if (!loadSuccess) {
        std::cerr << "ERROR::MESH::Failed to load OBJ file: " << path << std::endl;
        return nullptr;
    }

    // 假设 OBJ 文件只有一个 Mesh
    objl::Mesh curMesh = loader.LoadedMeshes[0];

    // 先收集所有顶点数据
    std::vector<Vertex> vertices;
    for (const auto& vertex : curMesh.Vertices) {
        vertices.emplace_back(
            glm::vec3(vertex.Position.X, vertex.Position.Y, vertex.Position.Z),
            glm::vec3(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z),
            glm::vec2(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y)
        );
    }

    // 根据indice, 创建图元Primitive
    auto indices = curMesh.Indices;
    for (size_t i = 0; i < indices.size(); i += 3) {
        // 确保不会越界
        if (i + 2 >= indices.size()) break;

        // 获取三个顶点的索引
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i+1];
        unsigned int idx2 = indices[i+2];

        // 确保索引有效
        if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
            std::cerr << "ERROR::MESH::Invalid vertex index in OBJ file" << std::endl;
            continue;
        }

        // 创建三角形并添加到builder中
        auto pri = Primitive(vertices[idx0], vertices[idx1], vertices[idx2]);
        builder->pri.push_back(pri);
    }

    return builder; // 返回智能指针
}

void BVHBuilder::Build() {
    // 转换操作
    std::vector<Primitive*> p_pri;
    p_pri.reserve(pri.size());
    for (auto& pri_i : pri) {
        p_pri.push_back(&pri_i);
    }

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    Kmeans *k = new Kmeans(1, 8, 5, p_pri, rank, num_procs);
    k->registerCallback(m_callback);

    // 创建表头
    if(rank == 0) {
        timer::create_k_means_header();
        std::cout << "[Log] K-means BVH Building..." << std::endl;
    }
    k->constructKaryTree(0);
    if(rank == 0) {
        std::cout << "[Log] K-means BVH Building Completed" << std::endl;
    }
}