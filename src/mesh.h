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

Mesh load_obj_mesh(const std::filesystem::path& path);
