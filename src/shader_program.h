#pragma once

#include <glad/gl.h>

#include <filesystem>

class ShaderProgram {
public:
    ShaderProgram(
        const std::filesystem::path& vertex_path,
        const std::filesystem::path& fragment_path);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&&) = delete;
    ShaderProgram& operator=(ShaderProgram&&) = delete;

    void use() const;
    void set_int(const char* name, int value) const;
    void set_vec2(const char* name, float x, float y) const;

private:
    GLuint program_id_ = 0;
};
