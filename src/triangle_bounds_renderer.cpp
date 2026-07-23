#include "triangle_bounds_renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <stdexcept>
#include <vector>

namespace {

std::vector<std::uint32_t> build_triangle_indices(const Mesh& mesh)
{
    std::vector<std::uint32_t> indices;
    indices.reserve(mesh.faces.size() * 3);
    for (const TriangleFace& face : mesh.faces) {
        indices.insert(
            indices.end(),
            face.indices.begin(),
            face.indices.end());
    }
    return indices;
}

} // namespace

TriangleBoundsRenderer::TriangleBoundsRenderer(
    const Mesh& mesh,
    const std::filesystem::path& shader_directory)
    : shader_(
          shader_directory / "triangle_bounds.vert",
          shader_directory / "triangle_bounds.geom",
          shader_directory / "triangle_bounds.frag")
{
    const std::vector<std::uint32_t> indices = build_triangle_indices(mesh);
    if (indices.empty()) {
        throw std::runtime_error(
            "Cannot draw triangle bounds for a mesh without faces.");
    }
    if (indices.size()
        > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error(
            "Mesh has too many indices for triangle bounds.");
    }
    triangle_index_count_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);
    glGenBuffers(1, &index_buffer_);
    if (vertex_array_ == 0 || vertex_buffer_ == 0 || index_buffer_ == 0) {
        throw std::runtime_error(
            "Could not create GPU triangle-bounds resources.");
    }

    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(glm::vec3)),
        mesh.vertices.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            indices.size() * sizeof(std::uint32_t)),
        indices.data(),
        GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

TriangleBoundsRenderer::~TriangleBoundsRenderer()
{
    if (index_buffer_ != 0) {
        glDeleteBuffers(1, &index_buffer_);
    }
    if (vertex_buffer_ != 0) {
        glDeleteBuffers(1, &vertex_buffer_);
    }
    if (vertex_array_ != 0) {
        glDeleteVertexArrays(1, &vertex_array_);
    }
}

std::size_t TriangleBoundsRenderer::render(
    const ViewportFit& fit,
    const TransformControls& transforms,
    const CameraControls& camera,
    const ProjectionControls& projection_controls) const
{
    const glm::mat4 viewport_fit =
        glm::translate(glm::mat4(1.0F), fit.translation)
        * glm::scale(glm::mat4(1.0F), glm::vec3(fit.uniform_scale));

    GLint previous_program = 0;
    GLint previous_vertex_array = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vertex_array);

    shader_.use();
    shader_.set_mat4("u_viewport_fit", viewport_fit);
    shader_.set_mat4(
        "u_local_transform",
        build_local_transform_matrix(transforms, fit));
    shader_.set_mat4(
        "u_world_transform",
        build_world_transform_matrix(transforms));
    shader_.set_mat4("u_view", build_camera_view_matrix(camera));
    shader_.set_mat4(
        "u_projection",
        build_projection_matrix(fit, projection_controls));
    glBindVertexArray(vertex_array_);
    glDrawElements(
        GL_TRIANGLES,
        triangle_index_count_,
        GL_UNSIGNED_INT,
        nullptr);

    glBindVertexArray(static_cast<GLuint>(previous_vertex_array));
    glUseProgram(static_cast<GLuint>(previous_program));
    return rectangle_count();
}

std::size_t TriangleBoundsRenderer::rectangle_count() const
{
    return static_cast<std::size_t>(triangle_index_count_) / 3;
}
