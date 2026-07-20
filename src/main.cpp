#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

constexpr int initial_width = 1280;
constexpr int initial_height = 720;
constexpr std::array<float, 4> clear_color {0.055F, 0.075F, 0.11F, 1.0F};

struct CommandLineOptions {
    bool validation_mode = false;
    bool valid = true;
};

CommandLineOptions parse_options(int argc, char* argv[])
{
    if (argc == 1) {
        return {};
    }

    if (argc == 3 && std::string_view(argv[1]) == "--validate"
        && std::string_view(argv[2]) == "foundation") {
        return {.validation_mode = true, .valid = true};
    }

    return {.validation_mode = false, .valid = false};
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
        std::cerr << "Usage: nanorender_opengl [--validate foundation]\n";
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
        options.validation_mode ? GLFW_FALSE : GLFW_TRUE);

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

    int exit_code = EXIT_SUCCESS;
    while (glfwWindowShouldClose(window) != GLFW_TRUE) {
        process_input(window);

        glClearColor(
            clear_color[0],
            clear_color[1],
            clear_color[2],
            clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        if (options.validation_mode) {
            if (validate_cleared_framebuffer(
                    framebuffer_width,
                    framebuffer_height)) {
                std::cout << "Foundation validation passed.\n";
            } else {
                std::cerr << "Foundation validation failed.\n";
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
