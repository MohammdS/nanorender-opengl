#include "mesh_renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <stdexcept>
#include <vector>

namespace {

std::vector<std::uint32_t> build_edge_indices(const Mesh& mesh)
{
    std::vector<std::uint32_t> indices;
    indices.reserve(mesh.faces.size() * 6);
    for (const TriangleFace& face : mesh.faces) {
        const auto& vertex = face.indices;
        indices.insert(
            indices.end(),
            {vertex[0], vertex[1], vertex[1], vertex[2], vertex[2], vertex[0]});
    }
    return indices;
}

} // namespace

MeshRenderer::MeshRenderer(
    const Mesh& mesh,
    const std::filesystem::path& shader_directory)
    : shader_(
          shader_directory / "wireframe.vert",
          shader_directory / "wireframe.frag")
{
    const std::vector<std::uint32_t> edge_indices = build_edge_indices(mesh);
    if (edge_indices.empty()) {
        throw std::runtime_error("Cannot render a mesh without face edges.");
    }
    if (edge_indices.size()
        > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error("Mesh has too many edge indices for OpenGL.");
    }
    edge_index_count_ = static_cast<GLsizei>(edge_indices.size());

    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);
    glGenBuffers(1, &index_buffer_);
    if (vertex_array_ == 0 || vertex_buffer_ == 0 || index_buffer_ == 0) {
        throw std::runtime_error("Could not create GPU mesh resources.");
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
            edge_indices.size() * sizeof(std::uint32_t)),
        edge_indices.data(),
        GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

MeshRenderer::~MeshRenderer()
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

void MeshRenderer::render(const ViewportFit& fit) const
{
    const glm::mat4 viewport_fit =
        glm::translate(glm::mat4(1.0F), fit.translation)
        * glm::scale(glm::mat4(1.0F), glm::vec3(fit.uniform_scale));
    const glm::mat4 projection = glm::ortho(
        0.0F,
        static_cast<float>(fit.viewport_width),
        0.0F,
        static_cast<float>(fit.viewport_height),
        -10000.0F,
        10000.0F);

    GLint previous_program = 0;
    GLint previous_vertex_array = 0;
    GLfloat previous_line_width = 1.0F;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vertex_array);
    glGetFloatv(GL_LINE_WIDTH, &previous_line_width);

    shader_.use();
    shader_.set_mat4("u_viewport_fit", viewport_fit);
    shader_.set_mat4("u_projection", projection);
    glBindVertexArray(vertex_array_);
    glLineWidth(1.0F);
    glDrawElements(
        GL_LINES,
        edge_index_count_,
        GL_UNSIGNED_INT,
        nullptr);

    glLineWidth(previous_line_width);
    glBindVertexArray(static_cast<GLuint>(previous_vertex_array));
    glUseProgram(static_cast<GLuint>(previous_program));
}

std::size_t MeshRenderer::edge_index_count() const
{
    return static_cast<std::size_t>(edge_index_count_);
}
