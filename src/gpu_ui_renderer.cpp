#include "gpu_ui_renderer.h"

#include "microui_atlas.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace {

void restore_capability(GLenum capability, GLboolean enabled)
{
    if (enabled == GL_TRUE) {
        glEnable(capability);
    } else {
        glDisable(capability);
    }
}

} // namespace

GpuUiRenderer::GpuUiRenderer(const std::filesystem::path& shader_directory)
    : shader_(shader_directory / "ui.vert", shader_directory / "ui.frag")
{
    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &vertex_buffer_);
    glGenTextures(1, &atlas_texture_);
    if (vertex_array_ == 0 || vertex_buffer_ == 0 || atlas_texture_ == 0) {
        throw std::runtime_error("Could not create GPU UI resources.");
    }

    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, texture_coordinate)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, color)));

    glBindTexture(GL_TEXTURE_2D, atlas_texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,
        nano_ui_atlas_width(),
        nano_ui_atlas_height(),
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        nano_ui_atlas_pixels());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GpuUiRenderer::~GpuUiRenderer()
{
    if (atlas_texture_ != 0) {
        glDeleteTextures(1, &atlas_texture_);
    }
    if (vertex_buffer_ != 0) {
        glDeleteBuffers(1, &vertex_buffer_);
    }
    if (vertex_array_ != 0) {
        glDeleteVertexArrays(1, &vertex_array_);
    }
}

int GpuUiRenderer::text_width(mu_Font, const char* text, int length)
{
    int width = 0;
    for (const unsigned char* character =
             reinterpret_cast<const unsigned char*>(text);
         *character != '\0' && length != 0;
         ++character) {
        if ((*character & 0xC0U) != 0x80U) {
            width += nano_ui_atlas_glyph_rect(*character).w;
            if (length > 0) {
                --length;
            }
        }
    }
    return width;
}

int GpuUiRenderer::text_height(mu_Font)
{
    return 18;
}

void GpuUiRenderer::push_quad(
    mu_Rect destination,
    mu_Rect source,
    mu_Color color)
{
    const float atlas_width = static_cast<float>(nano_ui_atlas_width());
    const float atlas_height = static_cast<float>(nano_ui_atlas_height());
    const float left = static_cast<float>(destination.x);
    const float top = static_cast<float>(destination.y);
    const float right = static_cast<float>(destination.x + destination.w);
    const float bottom = static_cast<float>(destination.y + destination.h);
    const float u0 = static_cast<float>(source.x) / atlas_width;
    const float v0 = static_cast<float>(source.y) / atlas_height;
    const float u1 = static_cast<float>(source.x + source.w) / atlas_width;
    const float v1 = static_cast<float>(source.y + source.h) / atlas_height;

    const auto make_vertex = [color](float x, float y, float u, float v) {
        Vertex vertex;
        vertex.position[0] = x;
        vertex.position[1] = y;
        vertex.texture_coordinate[0] = u;
        vertex.texture_coordinate[1] = v;
        vertex.color[0] = color.r;
        vertex.color[1] = color.g;
        vertex.color[2] = color.b;
        vertex.color[3] = color.a;
        return vertex;
    };

    vertices_.push_back(make_vertex(left, top, u0, v0));
    vertices_.push_back(make_vertex(right, top, u1, v0));
    vertices_.push_back(make_vertex(left, bottom, u0, v1));
    vertices_.push_back(make_vertex(left, bottom, u0, v1));
    vertices_.push_back(make_vertex(right, top, u1, v0));
    vertices_.push_back(make_vertex(right, bottom, u1, v1));
}

void GpuUiRenderer::push_text(
    const char* text,
    mu_Vec2 position,
    mu_Color color)
{
    mu_Rect destination {position.x, position.y, 0, 0};
    for (const unsigned char* character =
             reinterpret_cast<const unsigned char*>(text);
         *character != '\0';
         ++character) {
        if ((*character & 0xC0U) == 0x80U) {
            continue;
        }
        const mu_Rect source = nano_ui_atlas_glyph_rect(*character);
        destination.w = source.w;
        destination.h = source.h;
        push_quad(destination, source, color);
        destination.x += destination.w;
    }
}

void GpuUiRenderer::flush(int framebuffer_width, int framebuffer_height)
{
    if (vertices_.empty()) {
        return;
    }

    shader_.use();
    shader_.set_vec2(
        "u_viewport_size",
        static_cast<float>(framebuffer_width),
        static_cast<float>(framebuffer_height));
    shader_.set_int("u_atlas", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas_texture_);
    glBindVertexArray(vertex_array_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices_.size() * sizeof(Vertex)),
        vertices_.data(),
        GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
    vertices_.clear();
}

void GpuUiRenderer::set_clip(
    mu_Rect rect,
    int framebuffer_width,
    int framebuffer_height)
{
    flush(framebuffer_width, framebuffer_height);
    const int left = std::clamp(rect.x, 0, framebuffer_width);
    const int top = std::clamp(rect.y, 0, framebuffer_height);
    const int right =
        std::clamp(rect.x + rect.w, left, framebuffer_width);
    const int bottom =
        std::clamp(rect.y + rect.h, top, framebuffer_height);
    glScissor(
        left,
        framebuffer_height - bottom,
        right - left,
        bottom - top);
}

void GpuUiRenderer::render(
    mu_Context& context,
    int framebuffer_width,
    int framebuffer_height)
{
    if (framebuffer_width <= 0 || framebuffer_height <= 0) {
        return;
    }

    const GLboolean blend_enabled = glIsEnabled(GL_BLEND);
    const GLboolean cull_enabled = glIsEnabled(GL_CULL_FACE);
    const GLboolean depth_enabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean scissor_enabled = glIsEnabled(GL_SCISSOR_TEST);
    GLint previous_program = 0;
    GLint previous_vertex_array = 0;
    GLint previous_array_buffer = 0;
    GLint previous_active_texture = 0;
    GLint previous_texture = 0;
    GLint previous_scissor[4] {};
    GLint previous_blend_source_rgb = 0;
    GLint previous_blend_destination_rgb = 0;
    GLint previous_blend_source_alpha = 0;
    GLint previous_blend_destination_alpha = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vertex_array);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previous_array_buffer);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previous_active_texture);
    glGetIntegerv(GL_SCISSOR_BOX, previous_scissor);
    glGetIntegerv(GL_BLEND_SRC_RGB, &previous_blend_source_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &previous_blend_destination_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &previous_blend_source_alpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &previous_blend_destination_alpha);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    set_clip(
        mu_rect(0, 0, framebuffer_width, framebuffer_height),
        framebuffer_width,
        framebuffer_height);

    mu_Command* command = nullptr;
    while (mu_next_command(&context, &command) != 0) {
        switch (command->type) {
        case MU_COMMAND_RECT:
            push_quad(
                command->rect.rect,
                nano_ui_atlas_white_rect(),
                command->rect.color);
            break;
        case MU_COMMAND_TEXT:
            push_text(
                command->text.str,
                command->text.pos,
                command->text.color);
            break;
        case MU_COMMAND_ICON: {
            const mu_Rect source = nano_ui_atlas_icon_rect(command->icon.id);
            const int x = command->icon.rect.x
                + (command->icon.rect.w - source.w) / 2;
            const int y = command->icon.rect.y
                + (command->icon.rect.h - source.h) / 2;
            push_quad(
                mu_rect(x, y, source.w, source.h),
                source,
                command->icon.color);
            break;
        }
        case MU_COMMAND_CLIP:
            set_clip(
                command->clip.rect,
                framebuffer_width,
                framebuffer_height);
            break;
        default:
            break;
        }
    }
    flush(framebuffer_width, framebuffer_height);

    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous_texture));
    glActiveTexture(static_cast<GLenum>(previous_active_texture));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previous_array_buffer));
    glBindVertexArray(static_cast<GLuint>(previous_vertex_array));
    glUseProgram(static_cast<GLuint>(previous_program));
    glScissor(
        previous_scissor[0],
        previous_scissor[1],
        previous_scissor[2],
        previous_scissor[3]);
    glBlendFuncSeparate(
        static_cast<GLenum>(previous_blend_source_rgb),
        static_cast<GLenum>(previous_blend_destination_rgb),
        static_cast<GLenum>(previous_blend_source_alpha),
        static_cast<GLenum>(previous_blend_destination_alpha));
    restore_capability(GL_BLEND, blend_enabled);
    restore_capability(GL_CULL_FACE, cull_enabled);
    restore_capability(GL_DEPTH_TEST, depth_enabled);
    restore_capability(GL_SCISSOR_TEST, scissor_enabled);
}
