#pragma once

#include "camera.h"
#include "mesh.h"
#include "projection.h"
#include "shader_program.h"
#include "transform_controls.h"

#include <glad/gl.h>

#include <cstddef>
#include <filesystem>

class TriangleBoundsRenderer {
public:
    TriangleBoundsRenderer(
        const Mesh& mesh,
        const std::filesystem::path& shader_directory);
    ~TriangleBoundsRenderer();

    TriangleBoundsRenderer(const TriangleBoundsRenderer&) = delete;
    TriangleBoundsRenderer& operator=(const TriangleBoundsRenderer&) = delete;
    TriangleBoundsRenderer(TriangleBoundsRenderer&&) = delete;
    TriangleBoundsRenderer& operator=(TriangleBoundsRenderer&&) = delete;

    std::size_t render(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection) const;

    [[nodiscard]] std::size_t rectangle_count() const;

private:
    ShaderProgram shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLuint index_buffer_ = 0;
    GLsizei triangle_index_count_ = 0;
};
