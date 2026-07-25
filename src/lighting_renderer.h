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

struct ReflectionVectorCounts {
    std::size_t incoming = 0;
    std::size_t reflected = 0;

    [[nodiscard]] std::size_t total() const;
};

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

    std::size_t render_specular(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        const PointLight& light,
        const Material& material) const;

    std::size_t render_phong(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        const PointLight& light,
        const Material& material) const;

    ReflectionVectorCounts render_reflection_vectors(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        const PointLight& light,
        std::size_t face_limit = 3) const;

    [[nodiscard]] std::size_t triangle_count() const;

private:
    std::size_t render_flat_lighting(
        const ViewportFit& fit,
        const TransformControls& transforms,
        const CameraControls& camera,
        const ProjectionControls& projection,
        const PointLight& light,
        const Material& material,
        bool include_specular) const;

    ShaderProgram ambient_shader_;
    ShaderProgram flat_diffuse_shader_;
    ShaderProgram phong_shader_;
    ShaderProgram reflection_vector_shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLsizei vertex_count_ = 0;
};
