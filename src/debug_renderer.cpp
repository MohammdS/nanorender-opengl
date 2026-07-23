#include "debug_renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace {

using DebugVertex = std::pair<glm::vec3, glm::vec3>;

void append_line(
    std::vector<DebugVertex>& lines,
    const glm::vec3& start,
    const glm::vec3& end,
    const glm::vec3& color)
{
    lines.emplace_back(start, color);
    lines.emplace_back(end, color);
}

float debug_axis_length(const ViewportFit& fit)
{
    return std::max({fit.size.x, fit.size.y, fit.size.z}) * 0.45F;
}

} // namespace

std::size_t DebugLineCounts::total() const
{
    return local_axes + world_axes + bounding_box_edges;
}

DebugRenderer::DebugRenderer(const std::filesystem::path& shader_directory)
    : shader_(
          shader_directory / "debug.vert",
          shader_directory / "debug.frag")
{
    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);
    if (vertex_array_ == 0 || vertex_buffer_ == 0) {
        throw std::runtime_error("Could not create GPU debug-line resources.");
    }

    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, color)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

DebugRenderer::~DebugRenderer()
{
    if (vertex_buffer_ != 0) {
        glDeleteBuffers(1, &vertex_buffer_);
    }
    if (vertex_array_ != 0) {
        glDeleteVertexArrays(1, &vertex_array_);
    }
}

void DebugRenderer::draw_batch(
    const std::vector<Vertex>& vertices,
    const glm::mat4& viewport_fit,
    const glm::mat4& local_transform,
    const glm::mat4& world_transform,
    const glm::mat4& projection) const
{
    if (vertices.empty()) {
        return;
    }

    GLint previous_program = 0;
    GLint previous_vertex_array = 0;
    GLint previous_array_buffer = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vertex_array);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previous_array_buffer);

    shader_.use();
    shader_.set_mat4("u_viewport_fit", viewport_fit);
    shader_.set_mat4("u_local_transform", local_transform);
    shader_.set_mat4("u_world_transform", world_transform);
    shader_.set_mat4("u_projection", projection);
    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));

    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previous_array_buffer));
    glBindVertexArray(static_cast<GLuint>(previous_vertex_array));
    glUseProgram(static_cast<GLuint>(previous_program));
}

DebugLineCounts DebugRenderer::render(
    const ViewportFit& fit,
    const TransformControls& transforms,
    const DebugVisualControls& controls) const
{
    std::vector<DebugVertex> model_lines;
    std::vector<DebugVertex> world_lines;
    DebugLineCounts counts;
    const float axis_length = debug_axis_length(fit);

    if (controls.show_local_axes != 0) {
        const glm::vec3 origin = fit.center;
        append_line(
            model_lines,
            origin,
            origin + glm::vec3(axis_length, 0.0F, 0.0F),
            glm::vec3(1.0F, 0.2F, 0.2F));
        append_line(
            model_lines,
            origin,
            origin + glm::vec3(0.0F, axis_length, 0.0F),
            glm::vec3(0.2F, 1.0F, 0.35F));
        append_line(
            model_lines,
            origin,
            origin + glm::vec3(0.0F, 0.0F, axis_length),
            glm::vec3(0.25F, 0.5F, 1.0F));
        counts.local_axes = 3;
    }

    if (controls.show_bounding_box != 0) {
        const glm::vec3& minimum = fit.bounds.min;
        const glm::vec3& maximum = fit.bounds.max;
        const std::array corners {
            glm::vec3(minimum.x, minimum.y, minimum.z),
            glm::vec3(maximum.x, minimum.y, minimum.z),
            glm::vec3(maximum.x, maximum.y, minimum.z),
            glm::vec3(minimum.x, maximum.y, minimum.z),
            glm::vec3(minimum.x, minimum.y, maximum.z),
            glm::vec3(maximum.x, minimum.y, maximum.z),
            glm::vec3(maximum.x, maximum.y, maximum.z),
            glm::vec3(minimum.x, maximum.y, maximum.z),
        };
        constexpr std::array<std::array<std::size_t, 2>, 12> edges {{
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7},
        }};
        for (const auto& edge : edges) {
            append_line(
                model_lines,
                corners[edge[0]],
                corners[edge[1]],
                glm::vec3(1.0F, 0.72F, 0.12F));
        }
        counts.bounding_box_edges = edges.size();
    }

    if (controls.show_world_axes != 0) {
        const glm::vec3 origin(0.0F);
        append_line(
            world_lines,
            origin,
            origin + glm::vec3(axis_length, 0.0F, 0.0F),
            glm::vec3(0.7F, 0.05F, 0.05F));
        append_line(
            world_lines,
            origin,
            origin + glm::vec3(0.0F, axis_length, 0.0F),
            glm::vec3(0.05F, 0.62F, 0.15F));
        append_line(
            world_lines,
            origin,
            origin + glm::vec3(0.0F, 0.0F, axis_length),
            glm::vec3(0.1F, 0.3F, 0.8F));
        counts.world_axes = 3;
    }

    const auto convert = [](const std::vector<DebugVertex>& source) {
        std::vector<Vertex> vertices;
        vertices.reserve(source.size());
        for (const auto& [position, color] : source) {
            vertices.push_back({position, color});
        }
        return vertices;
    };
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
    const glm::mat4 identity(1.0F);
    draw_batch(
        convert(model_lines),
        viewport_fit,
        build_local_transform_matrix(transforms, fit),
        build_world_transform_matrix(transforms),
        projection);
    draw_batch(
        convert(world_lines),
        viewport_fit,
        identity,
        identity,
        projection);
    return counts;
}
