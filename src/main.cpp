#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "mesh.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr int initial_width = 1280;
constexpr int initial_height = 720;
constexpr std::array<float, 4> clear_color {0.055F, 0.075F, 0.11F, 1.0F};

enum class ValidationMode {
    none,
    foundation,
    hw2_task1,
};

struct CommandLineOptions {
    ValidationMode validation = ValidationMode::none;
    bool valid = true;
};

CommandLineOptions parse_options(int argc, char* argv[])
{
    if (argc == 1) {
        return {};
    }

    if (argc == 3 && std::string_view(argv[1]) == "--validate") {
        const std::string_view feature = argv[2];
        if (feature == "foundation") {
            return {.validation = ValidationMode::foundation, .valid = true};
        }
        if (feature == "hw2-task1") {
            return {.validation = ValidationMode::hw2_task1, .valid = true};
        }
    }

    return {.validation = ValidationMode::none, .valid = false};
}

void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW error " << error << ": " << description << '\n';
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

std::filesystem::path find_model_path(const char* executable_argument)
{
    const std::filesystem::path executable_directory =
        std::filesystem::absolute(executable_argument).parent_path();
    const std::array candidates {
        std::filesystem::current_path() / "models" / "task1_tetrahedron.obj",
        executable_directory / "models" / "task1_tetrahedron.obj",
    };

    for (const std::filesystem::path& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }
    }
    throw std::runtime_error("Could not find models/task1_tetrahedron.obj");
}

bool validate_hw2_task1(const Mesh& mesh)
{
    bool indices_are_valid = true;
    for (const TriangleFace& face : mesh.faces) {
        for (const std::uint32_t index : face.indices) {
            indices_are_valid = indices_are_valid
                && index < mesh.vertices.size();
        }
    }

    std::cout << "HW2 Task 1 OBJ loader: vertices=" << mesh.vertices.size()
              << " faces=" << mesh.faces.size()
              << " valid_indices=" << (indices_are_valid ? "yes" : "no")
              << '\n';
    return mesh.vertices.size() == 4 && mesh.faces.size() == 4
        && indices_are_valid;
}

bool validate_cleared_framebuffer(int width, int height)
{
    std::array<unsigned char, 4> pixel {};
    glReadPixels(
        width / 2,
        height / 2,
        1,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixel.data());

    std::array<int, 3> expected {};
    for (std::size_t channel = 0; channel < expected.size(); ++channel) {
        expected[channel] =
            static_cast<int>(std::lround(clear_color[channel] * 255.0F));
    }

    constexpr int tolerance = 2;
    const bool color_matches =
        std::abs(static_cast<int>(pixel[0]) - expected[0]) <= tolerance
        && std::abs(static_cast<int>(pixel[1]) - expected[1]) <= tolerance
        && std::abs(static_cast<int>(pixel[2]) - expected[2]) <= tolerance;

    std::cout << "Foundation framebuffer: " << width << 'x' << height
              << ", center rgb(" << static_cast<int>(pixel[0]) << ", "
              << static_cast<int>(pixel[1]) << ", "
              << static_cast<int>(pixel[2]) << ")\n";

    return color_matches && glGetError() == GL_NO_ERROR;
}

} // namespace

int main(int argc, char* argv[])
{
    const CommandLineOptions options = parse_options(argc, argv);
    if (!options.valid) {
        std::cerr << "Usage: nanorender_opengl "
                     "[--validate foundation|hw2-task1]\n";
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW.\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(
        GLFW_VISIBLE,
        options.validation == ValidationMode::none ? GLFW_TRUE : GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(
        initial_width,
        initial_height,
        "NanoRender OpenGL",
        nullptr,
        nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create the OpenGL window.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    const int loaded_version =
        gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
    if (loaded_version == 0) {
        std::cerr << "Failed to load OpenGL functions.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(loaded_version) << '.'
              << GLAD_VERSION_MINOR(loaded_version) << " initialized.\n";

    Mesh mesh;
    std::filesystem::path mesh_path;
    try {
        mesh_path = find_model_path(argv[0]);
        mesh = load_obj_mesh(mesh_path);
    } catch (const std::exception& exception) {
        std::cerr << "Mesh loading failed: " << exception.what() << '\n';
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "Loaded OBJ " << mesh_path.filename().string()
              << ": vertices=" << mesh.vertices.size()
              << " faces=" << mesh.faces.size() << '\n';
    if (options.validation == ValidationMode::none) {
        const std::string title = "NanoRender OpenGL | "
            + mesh_path.filename().string() + " | vertices="
            + std::to_string(mesh.vertices.size()) + " faces="
            + std::to_string(mesh.faces.size());
        glfwSetWindowTitle(window, title.c_str());
    }

    int exit_code = EXIT_SUCCESS;
    while (glfwWindowShouldClose(window) != GLFW_TRUE) {
        process_input(window);

        glClearColor(
            clear_color[0],
            clear_color[1],
            clear_color[2],
            clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        if (options.validation != ValidationMode::none) {
            const bool passed = options.validation == ValidationMode::foundation
                ? validate_cleared_framebuffer(
                      framebuffer_width,
                      framebuffer_height)
                : validate_hw2_task1(mesh);
            const char* validation_name =
                options.validation == ValidationMode::foundation
                ? "Foundation"
                : "HW2 Task 1";
            if (passed) {
                std::cout << validation_name << " validation passed.\n";
            } else {
                std::cerr << validation_name << " validation failed.\n";
                exit_code = EXIT_FAILURE;
            }
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
}
