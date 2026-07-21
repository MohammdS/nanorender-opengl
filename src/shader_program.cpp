#include "shader_program.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string read_text_file(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open shader: " + path.string());
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string shader_log(GLuint shader)
{
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(static_cast<std::size_t>(std::max(length, 1)), '\0');
    glGetShaderInfoLog(shader, length, nullptr, log.data());
    return std::string(log.data());
}

std::string program_log(GLuint program)
{
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(static_cast<std::size_t>(std::max(length, 1)), '\0');
    glGetProgramInfoLog(program, length, nullptr, log.data());
    return std::string(log.data());
}

GLuint compile_shader(
    GLenum type,
    const std::filesystem::path& path,
    const char* stage_name)
{
    const std::string source = read_text_file(path);
    const char* source_pointer = source.c_str();
    const GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw std::runtime_error(
            std::string("Could not create ") + stage_name + " shader.");
    }

    glShaderSource(shader, 1, &source_pointer, nullptr);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        const std::string log = shader_log(shader);
        glDeleteShader(shader);
        throw std::runtime_error(
            std::string(stage_name) + " shader compilation failed:\n" + log);
    }
    return shader;
}

GLint uniform_location(GLuint program, const char* name)
{
    const GLint location = glGetUniformLocation(program, name);
    if (location < 0) {
        throw std::runtime_error(
            std::string("Shader uniform was not found: ") + name);
    }
    return location;
}

} // namespace

ShaderProgram::ShaderProgram(
    const std::filesystem::path& vertex_path,
    const std::filesystem::path& fragment_path)
{
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    try {
        vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_path, "Vertex");
        fragment_shader =
            compile_shader(GL_FRAGMENT_SHADER, fragment_path, "Fragment");
        program_id_ = glCreateProgram();
        if (program_id_ == 0) {
            throw std::runtime_error("Could not create shader program.");
        }
        glAttachShader(program_id_, vertex_shader);
        glAttachShader(program_id_, fragment_shader);
        glLinkProgram(program_id_);

        GLint linked = GL_FALSE;
        glGetProgramiv(program_id_, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            throw std::runtime_error(
                "Shader program linking failed:\n" + program_log(program_id_));
        }
    } catch (...) {
        if (program_id_ != 0) {
            glDeleteProgram(program_id_);
            program_id_ = 0;
        }
        if (fragment_shader != 0) {
            glDeleteShader(fragment_shader);
        }
        if (vertex_shader != 0) {
            glDeleteShader(vertex_shader);
        }
        throw;
    }

    glDetachShader(program_id_, vertex_shader);
    glDetachShader(program_id_, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

ShaderProgram::~ShaderProgram()
{
    if (program_id_ != 0) {
        glDeleteProgram(program_id_);
    }
}

void ShaderProgram::use() const
{
    glUseProgram(program_id_);
}

void ShaderProgram::set_int(const char* name, int value) const
{
    glUniform1i(uniform_location(program_id_, name), value);
}

void ShaderProgram::set_vec2(const char* name, float x, float y) const
{
    glUniform2f(uniform_location(program_id_, name), x, y);
}
