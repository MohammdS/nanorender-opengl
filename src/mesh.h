#pragma once

#include <glm/vec3.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

struct TriangleFace {
    std::array<std::uint32_t, 3> indices {};
};

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<TriangleFace> faces;
};

struct MeshBounds {
    glm::vec3 min {0.0F};
    glm::vec3 max {0.0F};
};

struct ViewportFit {
    MeshBounds bounds;
    glm::vec3 center {0.0F};
    glm::vec3 size {0.0F};
    float uniform_scale = 1.0F;
    glm::vec3 translation {0.0F};
    int viewport_width = 0;
    int viewport_height = 0;
};

Mesh load_obj_mesh(const std::filesystem::path& path);
MeshBounds calculate_mesh_bounds(const Mesh& mesh);
ViewportFit calculate_viewport_fit(
    const Mesh& mesh,
    int viewport_width,
    int viewport_height,
    float viewport_coverage = 0.4F);
glm::vec3 apply_viewport_fit(
    const glm::vec3& vertex,
    const ViewportFit& fit);
