#include "filled_triangle_renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

struct FilledVertex {
    glm::vec3 position {0.0F};
    glm::vec3 barycentric {0.0F};
    glm::vec3 face_color {1.0F};
};

glm::vec3 stable_face_color(std::size_t face_index)
{
    std::uint32_t value =
        static_cast<std::uint32_t>(face_index) * 1664525U + 1013904223U;
    const auto component = [&value]() {
        value = value * 1664525U + 1013904223U;
        const float normalized =
            static_cast<float>((value >> 16U) & 0xFFU) / 255.0F;
        return 0.25F + normalized * 0.70F;
    };
    return {component(), component(), component()};
}

std::vector<FilledVertex> build_filled_vertices(const Mesh& mesh)
{
    constexpr std::array<glm::vec3, 3> barycentric_coordinates {
        glm::vec3(1.0F, 0.0F, 0.0F),
        glm::vec3(0.0F, 1.0F, 0.0F),
        glm::vec3(0.0F, 0.0F, 1.0F),
    };
    std::vector<FilledVertex> vertices;
    vertices.reserve(mesh.faces.size() * 3);
    for (std::size_t face_index = 0;
         face_index < mesh.faces.size();
         ++face_index) {
        const TriangleFace& face = mesh.faces[face_index];
        const glm::vec3 color = stable_face_color(face_index);
        for (std::size_t corner = 0; corner < face.indices.size(); ++corner) {
            const std::uint32_t vertex_index = face.indices[corner];
            if (vertex_index >= mesh.vertices.size()) {
                throw std::runtime_error(
                    "Cannot fill a triangle with an invalid vertex index.");
            }
            vertices.push_back(
                {mesh.vertices[vertex_index],
                 barycentric_coordinates[corner],
                 color});
        }
    }
    return vertices;
}

} // namespace

FilledTriangleRenderer::FilledTriangleRenderer(
    const Mesh& mesh,
    const std::filesystem::path& shader_directory)
    : shader_(
          shader_directory / "filled_triangle.vert",
          shader_directory / "filled_triangle.frag")
{
    const std::vector<FilledVertex> vertices = build_filled_vertices(mesh);
    if (vertices.empty()) {
        throw std::runtime_error(
            "Cannot fill triangles for a mesh without faces.");
    }
    if (vertices.size()
        > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error("Mesh has too many filled vertices.");
    }
    vertex_count_ = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);
    if (vertex_array_ == 0 || vertex_buffer_ == 0) {
        throw std::runtime_error(
            "Could not create GPU filled-triangle resources.");
    }

    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(FilledVertex)),
        vertices.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(FilledVertex),
        reinterpret_cast<const void*>(offsetof(FilledVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(FilledVertex),
        reinterpret_cast<const void*>(offsetof(FilledVertex, barycentric)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(FilledVertex),
        reinterpret_cast<const void*>(offsetof(FilledVertex, face_color)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

FilledTriangleRenderer::~FilledTriangleRenderer()
{
    if (vertex_buffer_ != 0) {
        glDeleteBuffers(1, &vertex_buffer_);
    }
    if (vertex_array_ != 0) {
        glDeleteVertexArrays(1, &vertex_array_);
    }
}

std::size_t FilledTriangleRenderer::render(
    const ViewportFit& fit,
    const TransformControls& transforms,
    const CameraControls& camera,
    const ProjectionControls& projection_controls,
    bool show_barycentric,
    bool enable_depth_test,
    bool show_depth) const
{
    const glm::mat4 viewport_fit =
        glm::translate(glm::mat4(1.0F), fit.translation)
        * glm::scale(glm::mat4(1.0F), glm::vec3(fit.uniform_scale));

    GLint previous_program = 0;
    GLint previous_vertex_array = 0;
    GLint previous_depth_function = GL_LESS;
    GLboolean previous_depth_mask = GL_TRUE;
    const GLboolean depth_test_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vertex_array);
    glGetIntegerv(GL_DEPTH_FUNC, &previous_depth_function);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previous_depth_mask);

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
    shader_.set_int("u_show_barycentric", show_barycentric ? 1 : 0);
    shader_.set_int("u_show_depth", show_depth ? 1 : 0);
    const float near_plane = std::max(
        projection_controls.near_plane,
        0.01F);
    const float far_plane = std::max(
        projection_controls.far_plane,
        near_plane + 1.0F);
    shader_.set_float("u_near_plane", near_plane);
    shader_.set_float("u_far_plane", far_plane);
    if (enable_depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glBindVertexArray(vertex_array_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

    glDepthFunc(static_cast<GLenum>(previous_depth_function));
    glDepthMask(previous_depth_mask);
    if (depth_test_was_enabled == GL_TRUE) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glBindVertexArray(static_cast<GLuint>(previous_vertex_array));
    glUseProgram(static_cast<GLuint>(previous_program));
    return triangle_count();
}

std::size_t FilledTriangleRenderer::triangle_count() const
{
    return static_cast<std::size_t>(vertex_count_) / 3;
}
