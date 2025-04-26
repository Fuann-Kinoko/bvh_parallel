#include "kmeans.hpp"
#include "bbox.hpp"
#include "primitive.h"
#include "../visualization/BVH.h"
#include "construction/timer.hpp"

#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <chrono> // 用于计时
#include <fstream> // 用于写入CSV文件
#include <mpi.h> // MPI

using namespace std;
#define maxLeafNum 4

static int UNIQUE_ID = 0;

Kmeans::Kmeans(size_t iterCount, size_t K, size_t P, vector<Primitive*> primitives, int rank, int num_procs)
//迭代次数、聚类数、随机点数、集几何体
{
    m_iterations = iterCount;
    m_K = K;
    m_P = P;
    this->rank = rank;
    this->num_procs = num_procs;
    this->unique_id = UNIQUE_ID++;
    this->primitives = primitives;
    children_existence = std::vector<int>(m_K, 1);
    cluster = new Cluster[m_K];
    children = new Kmeans*[m_K];

    if(rank == 0) {
        BoundingBox cur_world;
        for (size_t i = 0; i < primitives.size(); ++i) {
            cur_world.expand(primitives[i]->get_bbox());
        }
        world = cur_world;

        std::vector<BoundingBox> kCentroids = getRandCentroidsOnMesh(m_K, m_P);
        for (size_t i = 0; i < m_K; ++i) {
            cluster[i].representive = kCentroids[i];
        }
    }
}

Kmeans::~Kmeans()
{
    delete[] cluster;
}

// 随机选择在模型上的点而不是空间内的点
vector<BoundingBox> Kmeans::getRandCentroidsOnMesh(int k, int p)
{
    vector<BoundingBox> kCentroids;
    size_t idx_primitive;

    // 第一个随机点
    srand((unsigned)time(NULL));
    idx_primitive = rand() % primitives.size();
    kCentroids.push_back(primitives[idx_primitive]->get_bbox());

    srand((unsigned)time(NULL));
    // 选取之后k-1个点
    for (int i = 1; i < k; ++i) {
        // 随机p个点
        std::vector<BoundingBox> tempP;
        for (int j = 0; j < p; j++) {
            idx_primitive = rand() % primitives.size();
            tempP.push_back(primitives[idx_primitive]->get_bbox());
        }

        // 选取与之前算出的点距离最远的点作为下一个representive
        int index = 0;
        float maxDistance = -1.0f;
        for (int k = 0; k < p; k++)
            for (int q = 0; q < kCentroids.size(); q++) {
                BoundingBox bb;
                bb.expand(tempP[k]);
                bb.expand(kCentroids[q]);
                float distance = glm::length(bb.extent);
                if (distance > maxDistance) {
                    maxDistance = distance;
                    index = k;
                }
            }
        kCentroids.push_back(tempP[index]);
    }
    return kCentroids;
}

float Kmeans::calDistance(BoundingBox b1, BoundingBox b2)
{
    double min_value = glm::length(b1.min - b2.min);
    double max_value = glm::length(b1.max - b2.max);
    double res = min_value + max_value;
    return res;
}


void Kmeans::registerCallback(std::function<void (const BoundingBox, const bool)> func) {
    callback_func = func;
}

// Parallel BVH Construction using k-means Clustering
// https://meistdan.github.io/publications/kmeans/paper.pdf
void Kmeans::constructKaryTree(int depth)
{
    // printf("[id=%d,rank=%d] ==depth=%d== start constructKaryTree\n", this->unique_id, rank, depth);
    // 计时开始
    auto start_time = std::chrono::high_resolution_clock::now();

    // 构造本层结构
    // this->run();
    this->runMPI();

    // 计时结束
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    if(this->rank == 0) {
        timer::write_k_means_time(this->unique_id, depth, elapsed_us.count());
    }

    // 判定子节点是否是叶子节点
    if(this->rank == 0) {
        for (size_t i = 0; i < m_K; i++) {
            // 叶子cluster 该cluster[i]对应的children为NULL
            if (cluster[i].indexOfPrimitives.size() < maxLeafNum * m_K) {
                children_existence[i] = 0;
                if(callback_func) {
                    callback_func(cluster[i].world, true);
                }
                continue;
            }
        }
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(this->children_existence.data(), m_K, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Barrier(MPI_COMM_WORLD);
    int child_cnt = 0;
    for(int i=0;i<m_K;++i) child_cnt += children_existence[i];
    // MPI_Bcast(local_cluster_data.data(), m_K * MARSHAL_BBOX_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);
    // printf("[id=%d,rank=%d] detect children: %d\n", this->unique_id, rank, child_cnt);
    // 构造本层完成，通过callback通知visualization已经发生改变
    if(this->rank == 0) {
        if(callback_func) {
            callback_func(this->world, false);
        }
    }
    // 对本层中的每个cluster循环构造下层
    for (size_t i = 0; i < m_K; i++) {
        // 跳过叶子children
        if (!children_existence[i])
            continue;
        // 否则DFS
        vector<Primitive *> pTemp;
        if(this->rank == 0) {
            for (size_t p = 0; p < cluster[i].indexOfPrimitives.size(); ++p) {
                pTemp.push_back(primitives[cluster[i].indexOfPrimitives[p]]);
            }
        }
        children[i] = new Kmeans(m_iterations, m_K, m_P, pTemp, this->rank, this->num_procs);
        if(this->rank == 0) {
            if(callback_func) {
                children[i]->registerCallback(callback_func);
            }
        }
        children[i]->constructKaryTree(depth + 1);
    }
}

void Kmeans::run()
{
    for (size_t iter = 0; iter < m_iterations; ++iter) {
        // 更新representives cluster重新分配
        for (size_t i = 0; i < m_K; ++i) {
            if (iter != 0) {
                cluster[i].updateRepresentive();
            }
            cluster[i].reset();
            cluster[i].indexOfPrimitives.clear();
        }

        // 计算距离 分配至最近的cluster
        for (size_t idx_primitives = 0; idx_primitives < primitives.size(); ++idx_primitives) {
            // 用以记录最近的cluster
            size_t index = 0;
            double minDistance = numeric_limits<double>::max();
            BoundingBox temp = primitives[idx_primitives]->get_bbox();
            for (size_t idx_clusters = 0; idx_clusters < m_K; ++idx_clusters) {
                double dist = calDistance(temp, cluster[idx_clusters].representive);
                if (dist < minDistance) {
                    minDistance = dist;
                    index = idx_clusters;
                }
            }
            // allocate to correct cluster;
            cluster[index].add(idx_primitives, primitives[idx_primitives]->get_bbox());
        }
    }
    // print();
}

#if SIZE_MAX == UCHAR_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
   #error "what is happening here?"
#endif

#define MARSHAL_BBOX_SIZE 9
// Serialize a BoundingBox to a float array
std::vector<float> serialize_bbox(const BoundingBox& bbox) {
    std::vector<float> data(9); // 3 vec3s = 9 floats
    data[0] = bbox.max.x; data[1] = bbox.max.y; data[2] = bbox.max.z;
    data[3] = bbox.min.x; data[4] = bbox.min.y; data[5] = bbox.min.z;
    data[6] = bbox.extent.x; data[7] = bbox.extent.y; data[8] = bbox.extent.z;
    return data;
}

// Deserialize a float array to BoundingBox
BoundingBox deserialize_bbox(const float* data) {
    BoundingBox bbox;
    bbox.max = glm::vec3(data[0], data[1], data[2]);
    bbox.min = glm::vec3(data[3], data[4], data[5]);
    bbox.extent = glm::vec3(data[6], data[7], data[8]);
    return bbox;
}

void Kmeans::runMPI() {
    // printf("[id=%d,rank=%d] starting running for K-means\n", this->unique_id, rank);

    // Each process works on a subset of primitives
    size_t total_size;
    if(rank == 0) {
        total_size = primitives.size();
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&total_size, 1, my_MPI_SIZE_T, 0, MPI_COMM_WORLD);
    size_t local_n = total_size / num_procs;
    size_t start = rank * local_n;
    size_t end = (rank == num_procs - 1) ? total_size : (start + local_n);

    // prepare local primitives
    std::vector<BoundingBox> local_primitive_bboxs;
    std::vector<float> local_data, cloned_data;
    local_primitive_bboxs.reserve(local_n);
    int *counts = new int[num_procs]; int *counts_marshal = new int[num_procs];
    int *displs = new int[num_procs]; int *displs_marshal = new int[num_procs];
    for(size_t i = 0; i < num_procs; ++i) {
        if(i == num_procs - 1)  counts[i] = total_size - local_n * i;
        else                    counts[i] = local_n;
        displs[i] = (i == 0) ? 0 : (displs[i-1] + counts[i-1]);
        counts_marshal[i] = counts[i] * MARSHAL_BBOX_SIZE;
        displs_marshal[i] = displs[i] * MARSHAL_BBOX_SIZE;
        // printf("[id=%d,rank=%d] i=%d, count = %d, displ = %d\n", this->unique_id, rank, i, counts[i], displs[i]);
    }
    local_n = counts[rank];
    if(rank == 0) {
        cloned_data.reserve(total_size * MARSHAL_BBOX_SIZE);
        for (size_t i = 0; i < total_size; ++i) {
            const auto tmp_bbox = primitives[i]->get_bbox();
            if(i < local_n) local_primitive_bboxs.push_back(tmp_bbox);
            auto marshaled = serialize_bbox(tmp_bbox);
            cloned_data.insert(cloned_data.end(), marshaled.begin(), marshaled.end());
        }
    }
    local_data.reserve(counts_marshal[rank]);
    // MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatterv(
        cloned_data.data(), counts_marshal, displs_marshal, MPI_FLOAT,
        local_data.data(), counts_marshal[rank], MPI_FLOAT,
        0, MPI_COMM_WORLD
    );

    if(rank != 0) {
        for (int i = 0; i < local_n; i++)
            local_primitive_bboxs.push_back(deserialize_bbox(&local_data[i*MARSHAL_BBOX_SIZE]));
    }

    // printf("[id=%d,rank=%d] total_size = %d, local_n = %d\n", this->unique_id, rank, toal_size, local_n);
    // printf("[id=%d,rank=%d] start primitive = %d, end primitive = %d, size = %d\n", this->unique_id, rank, start, end, local_primitive_bboxs.size());

    // sync cluster
    for (size_t iter = 0; iter < m_iterations; ++iter) {
        // Update cluster representatives (only root computes, then broadcasts)
        std::vector<BoundingBox> synced_cluster_representives;
        synced_cluster_representives.reserve(m_K);
        if (rank == 0) {
            for (size_t i = 0; i < m_K; ++i) {
                if (iter != 0) {
                    cluster[i].updateRepresentive();
                }
                synced_cluster_representives.push_back(cluster[i].representive);
                cluster[i].reset();
                cluster[i].indexOfPrimitives.clear();
            }
        }

        // Broadcast the updated cluster representatives to all processes
        std::vector<float> local_cluster_data;
        local_cluster_data.reserve(m_K * MARSHAL_BBOX_SIZE);
        if(rank == 0) {
            for (size_t i = 0; i < m_K; ++i) {
                auto marshaled = serialize_bbox(synced_cluster_representives[i]);
                local_cluster_data.insert(local_cluster_data.end(), marshaled.begin(), marshaled.end());
            }
        }
        // MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(local_cluster_data.data(), m_K * MARSHAL_BBOX_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);
        if(rank != 0) {
            for (int i = 0; i < m_K; i++)
                synced_cluster_representives.push_back(deserialize_bbox(&local_cluster_data[i*MARSHAL_BBOX_SIZE]));
        }
        // printf("[id=%d,rank=%d] received b_cast data, now cluster representives has: %d\n", this->unique_id, rank, synced_cluster_representives.size());
        for(int i = 0; i < m_K; ++i) {
            glm::vec3 *tt_max = &synced_cluster_representives[i].max;
            glm::vec3 *tt_min = &synced_cluster_representives[i].min;
            // printf("[id=%d,rank=%d] \tcluster_%d representive = {%.2f,%.2f,%.2f|%.2f,%.2f,%.2f}\n", this->unique_id, rank, i,
            //     tt_max->x, tt_max->y, tt_max->z, tt_min->x, tt_min->y, tt_min->z);
        }

        // Local computation: Assign primitives to the nearest cluster
        std::vector<size_t> local_assignments;
        local_assignments.reserve(local_n);

        // #pragma omp parallel for simd
        for (size_t local_primitive_idx = 0; local_primitive_idx < local_n; ++local_primitive_idx) {
            size_t cluster_index = 0;
            double minDistance = std::numeric_limits<double>::max();
            BoundingBox temp = local_primitive_bboxs[local_primitive_idx];

            // printf("[id=%d,rank=%d] \t\tlocal_%d = {%.2f,%.2f,%.2f|%.2f,%.2f,%.2f}\n", this->unique_id, rank, local_primitive_idx + start, temp.max.x, temp.max.y, temp.max.z, temp.min.x, temp.min.y, temp.min.z);
            for (size_t j = 0; j < m_K; ++j) {
                double dist = calDistance(temp, synced_cluster_representives[j]);
                // printf("[id=%d,rank=%d] \t\t\t local_%d -> dist_%d = %.2f\n", this->unique_id, rank, local_primitive_idx + start, j, dist);
                if (dist < minDistance) {
                    minDistance = dist;
                    cluster_index = j;
                }
            }
            // printf("[id=%d,rank=%d] \t\t\t local_%d => choosed %d\n", this->unique_id, rank, local_primitive_idx + start, cluster_index);
            local_assignments[local_primitive_idx] = cluster_index;
        }

        // Gather all assignments to root to update clusters
        // printf("[id=%d,rank=%d] \t\t local: assignments: ", this->unique_id, rank);
        // for(int i = 0; i < local_n; ++i)
        //     printf("%d ", local_assignments[i]);
        // printf("\n");
        std::vector<size_t> all_assignments;
        if (rank == 0) {
            all_assignments.reserve(total_size);
        }

        MPI_Gatherv(
            local_assignments.data(), counts[rank], my_MPI_SIZE_T,
            all_assignments.data(), counts, displs, my_MPI_SIZE_T,
            0, MPI_COMM_WORLD
        );
        // MPI_Barrier(MPI_COMM_WORLD);

        // if(rank == 0) {
        //     printf("[id=%d,rank=%d] \t\t all assignments: ", this->unique_id, rank);
        //     for(size_t i = 0; i < total_size; ++i)
        //         printf("%d ", all_assignments[i]);
        //     printf("\n");
        // }

        // Root updates the clusters
        if (rank == 0) {
            for (size_t idx_primitives = 0; idx_primitives < total_size; ++idx_primitives) {
                size_t cluster_idx = all_assignments[idx_primitives];
                cluster[cluster_idx].add(idx_primitives, local_primitive_bboxs[idx_primitives]);
            }
        }
    }

    // Final synchronization
    int run_end;
    if(rank == 0) {
        for(int i = 1; i < num_procs; ++i)
            MPI_Send(&run_end, 1, MPI_INT, i, this->unique_id, MPI_COMM_WORLD);
        // print();
    }
    else {
        MPI_Recv(&run_end, 1, MPI_INT, 0, this->unique_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    // MPI_Barrier(MPI_COMM_WORLD);
}

// void Kmeans::traverse_cluster(std::vector<float> *vertices, std::vector<float> *colors, std::vector<int> *indices) const {
//     glm::vec3 min = world.min;
//     glm::vec3 max = world.max;
//     glm::vec3 color(1);
//     bool is_leaf = true;
//     for (size_t child_i = 0; child_i < this->m_K; ++child_i) {
//         if (children_existence[child_i]) {
//             is_leaf = false;
//             break;
//         }
//     }
//     if(is_leaf) {
//         color = glm::vec3(0,1,0);
//     }
//     BVH::buildCube(vertices, colors, indices, min, max, color);
// }

void Kmeans::print() const
{
    cout << "**********************************" << endl;
    for (size_t i = 0; i < m_K; ++i) {
        cout << "#" << i << " Cluster: " << cluster[i].indexOfPrimitives.size() << endl;
    }
    cout << "**********************************" << endl;
    cout << endl;
}
