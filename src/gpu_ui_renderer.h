#pragma once

#include "shader_program.h"

extern "C" {
#include <microui.h>
}

#include <cstdint>
#include <filesystem>
#include <vector>

class GpuUiRenderer {
public:
    explicit GpuUiRenderer(const std::filesystem::path& shader_directory);
    ~GpuUiRenderer();

    GpuUiRenderer(const GpuUiRenderer&) = delete;
    GpuUiRenderer& operator=(const GpuUiRenderer&) = delete;
    GpuUiRenderer(GpuUiRenderer&&) = delete;
    GpuUiRenderer& operator=(GpuUiRenderer&&) = delete;

    void render(
        mu_Context& context,
        int framebuffer_width,
        int framebuffer_height);

    static int text_width(mu_Font font, const char* text, int length);
    static int text_height(mu_Font font);

private:
    struct Vertex {
        float position[2] {};
        float texture_coordinate[2] {};
        std::uint8_t color[4] {};
    };

    void push_quad(mu_Rect destination, mu_Rect source, mu_Color color);
    void push_text(const char* text, mu_Vec2 position, mu_Color color);
    void flush(int framebuffer_width, int framebuffer_height);
    void set_clip(
        mu_Rect rect,
        int framebuffer_width,
        int framebuffer_height);

    ShaderProgram shader_;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLuint atlas_texture_ = 0;
    std::vector<Vertex> vertices_;
};
