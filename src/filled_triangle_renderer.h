#pragma once

#include "camera.h"
#include "mesh.h"
#include "projection.h"
#include "shader_program.h"
#include "transform_controls.h"

#include <glad/gl.h>

#include <cstddef>
#include <filesystem>

class FilledTriangleRenderer {
public:
    FilledTriangleRenderer(
        const Mesh& mesh,
        const std::filesystem::path& shader_directory);
    ~FilledTriangleRenderer();

    FilledTriangleRenderer(const FilledTriangleRenderer&) = delete;
    FilledTriangleRenderer& operator=(const FilledTriangleRenderer&) = delete;
    FilledTriangleRenderer(FilledTriangleRenderer&&) = delete;
    FilledTriangleRenderer& operator=(FilledTriangleRenderer&&) = delete;

    std::size_t render(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        bool show_barycentric,
        bool enable_depth_test,
        bool show_depth) const;

    [[nodiscard]] std::size_t triangle_count() const;

private:
    ShaderProgram shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLsizei vertex_count_ = 0;
};
