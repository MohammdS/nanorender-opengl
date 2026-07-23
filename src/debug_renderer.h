#pragma once

#include "mesh.h"
#include "shader_program.h"
#include "transform_controls.h"

#include <glad/gl.h>

#include <cstddef>
#include <filesystem>
#include <vector>

struct DebugVisualControls {
    int show_local_axes = 0;
    int show_world_axes = 0;
    int show_bounding_box = 0;
};

struct DebugLineCounts {
    std::size_t local_axes = 0;
    std::size_t world_axes = 0;
    std::size_t bounding_box_edges = 0;

    [[nodiscard]] std::size_t total() const;
};

class DebugRenderer {
public:
    explicit DebugRenderer(const std::filesystem::path& shader_directory);
    ~DebugRenderer();

    DebugRenderer(const DebugRenderer&) = delete;
    DebugRenderer& operator=(const DebugRenderer&) = delete;
    DebugRenderer(DebugRenderer&&) = delete;
    DebugRenderer& operator=(DebugRenderer&&) = delete;

    DebugLineCounts render(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const DebugVisualControls& controls) const;

private:
    struct Vertex {
        glm::vec3 position {0.0F};
        glm::vec3 color {1.0F};
    };

    void draw_batch(
        const std::vector<Vertex>& vertices,
        const glm::mat4& viewport_fit,
        const glm::mat4& local_transform,
        const glm::mat4& world_transform,
        const glm::mat4& projection) const;

    ShaderProgram shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
};
