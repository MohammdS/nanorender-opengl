#pragma once

#include "camera.h"
#include "mesh.h"
#include "projection.h"
#include "shader_program.h"
#include "transform_controls.h"

#include <glad/gl.h>

#include <cstddef>
#include <filesystem>

class MeshRenderer {
public:
    MeshRenderer(
        const Mesh& mesh,
        const std::filesystem::path& shader_directory);
    ~MeshRenderer();

    MeshRenderer(const MeshRenderer&) = delete;
    MeshRenderer& operator=(const MeshRenderer&) = delete;
    MeshRenderer(MeshRenderer&&) = delete;
    MeshRenderer& operator=(MeshRenderer&&) = delete;

    void render(
        const ViewportFit& fit,
        const TransformControls& controls) const;
    void render(
        const ViewportFit& fit,
        const TransformControls& controls,
        const CameraControls& camera) const;
    void render(
        const ViewportFit& fit,
        const TransformControls& controls,
        const CameraControls& camera,
        const ProjectionControls& projection) const;

    [[nodiscard]] std::size_t edge_index_count() const;

private:
    ShaderProgram shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLuint index_buffer_ = 0;
    GLsizei edge_index_count_ = 0;
};
