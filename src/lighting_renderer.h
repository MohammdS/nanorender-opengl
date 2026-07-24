#pragma once

#include "camera.h"
#include "lighting.h"
#include "mesh.h"
#include "projection.h"
#include "shader_program.h"
#include "transform_controls.h"

#include <glad/gl.h>

#include <cstddef>
#include <filesystem>

class LightingRenderer {
public:
    LightingRenderer(
        const Mesh& mesh,
        const std::filesystem::path& shader_directory);
    ~LightingRenderer();

    LightingRenderer(const LightingRenderer&) = delete;
    LightingRenderer& operator=(const LightingRenderer&) = delete;
    LightingRenderer(LightingRenderer&&) = delete;
    LightingRenderer& operator=(LightingRenderer&&) = delete;

    std::size_t render_ambient(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        const PointLight& light,
        const Material& material) const;

    std::size_t render_flat_diffuse(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        const PointLight& light,
        const Material& material) const;

    [[nodiscard]] std::size_t triangle_count() const;

private:
    ShaderProgram ambient_shader_;
    ShaderProgram flat_diffuse_shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLsizei vertex_count_ = 0;
};
