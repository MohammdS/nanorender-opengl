#include "lighting_renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <stdexcept>
#include <vector>

namespace {

std::vector<glm::vec3> build_triangle_vertices(const Mesh& mesh)
{
    std::vector<glm::vec3> vertices;
    vertices.reserve(mesh.faces.size() * 3);
    for (const TriangleFace& face : mesh.faces) {
        for (const std::uint32_t index : face.indices) {
            if (index >= mesh.vertices.size()) {
                throw std::runtime_error(
                    "Cannot light a triangle with an invalid vertex index.");
            }
            vertices.push_back(mesh.vertices[index]);
        }
    }
    return vertices;
}

} // namespace

LightingRenderer::LightingRenderer(
    const Mesh& mesh,
    const std::filesystem::path& shader_directory)
    : shader_(
          shader_directory / "ambient_lighting.vert",
          shader_directory / "ambient_lighting.frag")
{
    const std::vector<glm::vec3> vertices = build_triangle_vertices(mesh);
    if (vertices.empty()) {
        throw std::runtime_error(
            "Cannot light a mesh without triangle faces.");
    }
    if (vertices.size()
        > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error("Mesh has too many lighting vertices.");
    }
    vertex_count_ = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);
    if (vertex_array_ == 0 || vertex_buffer_ == 0) {
        throw std::runtime_error(
            "Could not create GPU lighting resources.");
    }

    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3)),
        vertices.data(),
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

LightingRenderer::~LightingRenderer()
{
    if (vertex_buffer_ != 0) {
        glDeleteBuffers(1, &vertex_buffer_);
    }
    if (vertex_array_ != 0) {
        glDeleteVertexArrays(1, &vertex_array_);
    }
}

std::size_t LightingRenderer::render_ambient(
    const ViewportFit& fit,
    const TransformControls& transforms,
    const CameraControls& camera,
    const ProjectionControls& projection_controls,
    const PointLight& light,
    const Material& material) const
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
    shader_.set_vec3("u_light_ambient", light.ambient);
    shader_.set_vec3("u_material_ambient", material.ambient);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
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

std::size_t LightingRenderer::triangle_count() const
{
    return static_cast<std::size_t>(vertex_count_) / 3;
}
