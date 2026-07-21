#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "gpu_ui_renderer.h"
#include "mesh.h"
#include "mesh_renderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr int initial_width = 1280;
constexpr int initial_height = 720;
constexpr std::array<float, 4> clear_color {0.055F, 0.075F, 0.11F, 1.0F};

enum class ValidationMode {
    none,
    foundation,
    hw2_task1,
    hw2_task2,
    hw2_task3,
};

struct CommandLineOptions {
    ValidationMode validation = ValidationMode::none;
    bool valid = true;
};

struct UiInputState {
    bool previous_left_mouse = false;
    bool previous_info_key = false;
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
        if (feature == "hw2-task2") {
            return {.validation = ValidationMode::hw2_task2, .valid = true};
        }
        if (feature == "hw2-task3") {
            return {.validation = ValidationMode::hw2_task3, .valid = true};
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

bool update_ui_input(
    mu_Context& context,
    UiInputState& input,
    GLFWwindow* window,
    int framebuffer_width,
    int framebuffer_height)
{
    int window_width = 0;
    int window_height = 0;
    glfwGetWindowSize(window, &window_width, &window_height);

    double raw_mouse_x = 0.0;
    double raw_mouse_y = 0.0;
    glfwGetCursorPos(window, &raw_mouse_x, &raw_mouse_y);
    const double scale_x = window_width > 0
        ? static_cast<double>(framebuffer_width) / window_width
        : 1.0;
    const double scale_y = window_height > 0
        ? static_cast<double>(framebuffer_height) / window_height
        : 1.0;
    const int mouse_x =
        static_cast<int>(std::lround(raw_mouse_x * scale_x));
    const int mouse_y =
        static_cast<int>(std::lround(raw_mouse_y * scale_y));
    mu_input_mousemove(&context, mouse_x, mouse_y);

    const bool left_mouse =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (left_mouse && !input.previous_left_mouse) {
        mu_input_mousedown(
            &context,
            mouse_x,
            mouse_y,
            MU_MOUSE_LEFT);
    } else if (!left_mouse && input.previous_left_mouse) {
        mu_input_mouseup(
            &context,
            mouse_x,
            mouse_y,
            MU_MOUSE_LEFT);
    }
    input.previous_left_mouse = left_mouse;

    const bool info_key = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    const bool info_key_pressed = info_key && !input.previous_info_key;
    input.previous_info_key = info_key;
    return info_key_pressed;
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

std::filesystem::path find_shader_directory(const char* executable_argument)
{
    const std::filesystem::path executable_directory =
        std::filesystem::absolute(executable_argument).parent_path();
    const std::array candidates {
        std::filesystem::current_path() / "shaders",
        executable_directory / "shaders",
    };

    for (const std::filesystem::path& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate / "ui.vert")
            && std::filesystem::is_regular_file(candidate / "ui.frag")
            && std::filesystem::is_regular_file(candidate / "wireframe.vert")
            && std::filesystem::is_regular_file(candidate / "wireframe.frag")) {
            return candidate;
        }
    }
    throw std::runtime_error("Could not find the GPU shader files.");
}

void build_mesh_info_popup(
    mu_Context& context,
    const std::filesystem::path& mesh_path,
    const Mesh& mesh,
    const ViewportFit& fit,
    bool toggle_requested,
    bool& initialized)
{
    mu_begin(&context);
    mu_Container* container = mu_get_container(&context, "HW2 Mesh Info");
    if (!initialized) {
        container->open = 1;
        initialized = true;
    }
    if (toggle_requested) {
        container->open = container->open == 0 ? 1 : 0;
        if (container->open != 0) {
            mu_bring_to_front(&context, container);
        }
    }

    if (mu_begin_window(
            &context,
            "HW2 Mesh Info",
            mu_rect(24, 24, 430, 300))) {
        int full_width[] {-1};
        mu_layout_row(&context, 1, full_width, 0);

        char filename[160] {};
        char vertex_count[64] {};
        char face_count[64] {};
        char bounds_min[128] {};
        char bounds_max[128] {};
        char center[128] {};
        char size[128] {};
        char scale[96] {};
        std::snprintf(
            filename,
            sizeof(filename),
            "OBJ: %s",
            mesh_path.filename().string().c_str());
        std::snprintf(
            vertex_count,
            sizeof(vertex_count),
            "Vertices: %zu",
            mesh.vertices.size());
        std::snprintf(
            face_count,
            sizeof(face_count),
            "Faces: %zu",
            mesh.faces.size());
        std::snprintf(
            bounds_min,
            sizeof(bounds_min),
            "Bounds min: %.1f, %.1f, %.1f",
            fit.bounds.min.x,
            fit.bounds.min.y,
            fit.bounds.min.z);
        std::snprintf(
            bounds_max,
            sizeof(bounds_max),
            "Bounds max: %.1f, %.1f, %.1f",
            fit.bounds.max.x,
            fit.bounds.max.y,
            fit.bounds.max.z);
        std::snprintf(
            center,
            sizeof(center),
            "Center: %.1f, %.1f, %.1f",
            fit.center.x,
            fit.center.y,
            fit.center.z);
        std::snprintf(
            size,
            sizeof(size),
            "Size: %.1f, %.1f, %.1f",
            fit.size.x,
            fit.size.y,
            fit.size.z);
        std::snprintf(
            scale,
            sizeof(scale),
            "Viewport scale: %.1f px/unit",
            fit.uniform_scale);

        mu_label(&context, "Status: loaded");
        mu_label(&context, filename);
        mu_label(&context, vertex_count);
        mu_label(&context, face_count);
        mu_label(&context, bounds_min);
        mu_label(&context, bounds_max);
        mu_label(&context, center);
        mu_label(&context, size);
        mu_label(&context, scale);
        mu_label(&context, "Press I to hide or reopen this window.");
        mu_end_window(&context);
    }
    mu_end(&context);
}

bool popup_sample_differs_from_background(int width, int height)
{
    if (width <= 30 || height <= 60) {
        return false;
    }

    std::array<unsigned char, 4> pixel {};
    glReadPixels(
        30,
        height - 1 - 60,
        1,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixel.data());

    constexpr int tolerance = 8;
    for (std::size_t channel = 0; channel < 3; ++channel) {
        const int background = static_cast<int>(
            std::lround(clear_color[channel] * 255.0F));
        if (std::abs(static_cast<int>(pixel[channel]) - background)
            > tolerance) {
            return true;
        }
    }
    return false;
}

bool validate_hw2_task1(const Mesh& mesh, int width, int height)
{
    bool indices_are_valid = true;
    for (const TriangleFace& face : mesh.faces) {
        for (const std::uint32_t index : face.indices) {
            indices_are_valid = indices_are_valid
                && index < mesh.vertices.size();
        }
    }

    const bool popup_rendered =
        popup_sample_differs_from_background(width, height);
    std::cout << "HW2 Task 1 OBJ loader: vertices=" << mesh.vertices.size()
              << " faces=" << mesh.faces.size()
              << " valid_indices=" << (indices_are_valid ? "yes" : "no")
              << " popup_rendered=" << (popup_rendered ? "yes" : "no")
              << '\n';
    return mesh.vertices.size() == 4 && mesh.faces.size() == 4
        && indices_are_valid && popup_rendered
        && glGetError() == GL_NO_ERROR;
}

bool nearly_equal(float left, float right, float tolerance = 0.001F)
{
    return std::abs(left - right) <= tolerance;
}

bool validate_hw2_task2(
    const Mesh& mesh,
    const ViewportFit& fit)
{
    bool vertices_fit = true;
    for (const glm::vec3& vertex : mesh.vertices) {
        const glm::vec3 fitted = apply_viewport_fit(vertex, fit);
        vertices_fit = vertices_fit && fitted.x >= 0.0F
            && fitted.x <= static_cast<float>(fit.viewport_width)
            && fitted.y >= 0.0F
            && fitted.y <= static_cast<float>(fit.viewport_height);
    }

    const glm::vec3 fitted_center =
        apply_viewport_fit(fit.center, fit);
    const float expected_scale = 0.4F
        * static_cast<float>(
            std::min(fit.viewport_width, fit.viewport_height))
        / 2.0F;
    const bool expected_values =
        nearly_equal(fit.bounds.min.x, -1.0F)
        && nearly_equal(fit.bounds.min.y, -1.0F)
        && nearly_equal(fit.bounds.min.z, -1.0F)
        && nearly_equal(fit.bounds.max.x, 1.0F)
        && nearly_equal(fit.bounds.max.y, 1.0F)
        && nearly_equal(fit.bounds.max.z, 1.0F)
        && nearly_equal(fit.center.x, 0.0F)
        && nearly_equal(fit.center.y, 0.0F)
        && nearly_equal(fit.center.z, 0.0F)
        && nearly_equal(fit.uniform_scale, expected_scale)
        && nearly_equal(
            fitted_center.x,
            static_cast<float>(fit.viewport_width) * 0.5F)
        && nearly_equal(
            fitted_center.y,
            static_cast<float>(fit.viewport_height) * 0.5F);

    std::cout << std::fixed << std::setprecision(1)
              << "HW2 Task 2 viewport fit: bounds_min=("
              << fit.bounds.min.x << ',' << fit.bounds.min.y << ','
              << fit.bounds.min.z << ") bounds_max=(" << fit.bounds.max.x
              << ',' << fit.bounds.max.y << ',' << fit.bounds.max.z
              << ") center=(" << fit.center.x << ',' << fit.center.y << ','
              << fit.center.z << ") size=(" << fit.size.x << ','
              << fit.size.y << ',' << fit.size.z << ") scale="
              << fit.uniform_scale << " translation=(" << fit.translation.x
              << ',' << fit.translation.y << ',' << fit.translation.z
              << ") fitted=" << (vertices_fit ? "yes" : "no") << '\n'
              << std::defaultfloat;
    return vertices_fit && expected_values;
}

bool validate_hw2_task3(
    const Mesh& mesh,
    const MeshRenderer& renderer,
    int width,
    int height)
{
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(width)
            * static_cast<std::size_t>(height) * 3);
    glReadPixels(
        0,
        0,
        width,
        height,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pixels.data());

    std::size_t wireframe_pixels = 0;
    for (std::size_t offset = 0; offset < pixels.size(); offset += 3) {
        const unsigned char red = pixels[offset];
        const unsigned char green = pixels[offset + 1];
        const unsigned char blue = pixels[offset + 2];
        if (red >= 35 && red <= 70 && green >= 185 && blue >= 230) {
            ++wireframe_pixels;
        }
    }

    const std::size_t expected_indices = mesh.faces.size() * 6;
    const bool gpu_draw = wireframe_pixels >= 100;
    std::cout << "HW2 Task 3 indexed wireframe: vertices="
              << mesh.vertices.size() << " faces=" << mesh.faces.size()
              << " line_indices=" << renderer.edge_index_count()
              << " wireframe_pixels=" << wireframe_pixels
              << " gpu_draw=" << (gpu_draw ? "yes" : "no") << '\n';
    return renderer.edge_index_count() == expected_indices && gpu_draw
        && glGetError() == GL_NO_ERROR;
}

std::string make_window_title(
    const std::filesystem::path& mesh_path,
    const Mesh& mesh,
    const ViewportFit& fit)
{
    std::ostringstream title;
    title << "NanoRender OpenGL | " << mesh_path.filename().string()
          << " | vertices=" << mesh.vertices.size()
          << " faces=" << mesh.faces.size() << " | fit=" << std::fixed
          << std::setprecision(1) << fit.uniform_scale << " px/unit";
    return title.str();
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
                     "[--validate foundation|hw2-task1|hw2-task2|"
                     "hw2-task3]\n";
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
    ViewportFit viewport_fit;
    try {
        mesh_path = find_model_path(argv[0]);
        mesh = load_obj_mesh(mesh_path);
        viewport_fit = calculate_viewport_fit(
            mesh,
            framebuffer_width,
            framebuffer_height);
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
        const std::string title =
            make_window_title(mesh_path, mesh, viewport_fit);
        glfwSetWindowTitle(window, title.c_str());
    }

    std::unique_ptr<mu_Context> ui_context;
    std::unique_ptr<GpuUiRenderer> ui_renderer;
    std::unique_ptr<MeshRenderer> mesh_renderer;
    if (options.validation == ValidationMode::none
        || options.validation == ValidationMode::hw2_task3) {
        try {
            mesh_renderer = std::make_unique<MeshRenderer>(
                mesh,
                find_shader_directory(argv[0]));
        } catch (const std::exception& exception) {
            std::cerr << "GPU mesh initialization failed: "
                      << exception.what() << '\n';
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }
    }
    if (options.validation != ValidationMode::foundation) {
        try {
            ui_context = std::make_unique<mu_Context>();
            mu_init(ui_context.get());
            ui_context->text_width = GpuUiRenderer::text_width;
            ui_context->text_height = GpuUiRenderer::text_height;
            ui_renderer = std::make_unique<GpuUiRenderer>(
                find_shader_directory(argv[0]));
        } catch (const std::exception& exception) {
            std::cerr << "GPU UI initialization failed: "
                      << exception.what() << '\n';
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }
    }

    UiInputState ui_input;
    bool popup_initialized = false;
    int exit_code = EXIT_SUCCESS;
    while (glfwWindowShouldClose(window) != GLFW_TRUE) {
        process_input(window);

        int current_framebuffer_width = 0;
        int current_framebuffer_height = 0;
        glfwGetFramebufferSize(
            window,
            &current_framebuffer_width,
            &current_framebuffer_height);
        if (current_framebuffer_width > 0 && current_framebuffer_height > 0
            && (current_framebuffer_width != framebuffer_width
                || current_framebuffer_height != framebuffer_height)) {
            framebuffer_width = current_framebuffer_width;
            framebuffer_height = current_framebuffer_height;
            viewport_fit = calculate_viewport_fit(
                mesh,
                framebuffer_width,
                framebuffer_height);
            if (options.validation == ValidationMode::none) {
                const std::string title =
                    make_window_title(mesh_path, mesh, viewport_fit);
                glfwSetWindowTitle(window, title.c_str());
            }
        }

        bool toggle_popup = false;
        if (ui_context != nullptr) {
            if (options.validation == ValidationMode::none) {
                toggle_popup = update_ui_input(
                    *ui_context,
                    ui_input,
                    window,
                    framebuffer_width,
                    framebuffer_height);
            }
            build_mesh_info_popup(
                *ui_context,
                mesh_path,
                mesh,
                viewport_fit,
                toggle_popup,
                popup_initialized);
        }

        glClearColor(
            clear_color[0],
            clear_color[1],
            clear_color[2],
            clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        if (mesh_renderer != nullptr) {
            mesh_renderer->render(viewport_fit);
        }

        if (ui_renderer != nullptr) {
            ui_renderer->render(
                *ui_context,
                framebuffer_width,
                framebuffer_height);
        }

        if (options.validation != ValidationMode::none) {
            bool passed = false;
            const char* validation_name = nullptr;
            if (options.validation == ValidationMode::foundation) {
                passed = validate_cleared_framebuffer(
                    framebuffer_width,
                    framebuffer_height);
                validation_name = "Foundation";
            } else if (options.validation == ValidationMode::hw2_task1) {
                passed = validate_hw2_task1(
                    mesh,
                    framebuffer_width,
                    framebuffer_height);
                validation_name = "HW2 Task 1";
            } else if (options.validation == ValidationMode::hw2_task2) {
                passed = validate_hw2_task2(mesh, viewport_fit);
                validation_name = "HW2 Task 2";
            } else {
                passed = validate_hw2_task3(
                    mesh,
                    *mesh_renderer,
                    framebuffer_width,
                    framebuffer_height);
                validation_name = "HW2 Task 3";
            }
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

    ui_renderer.reset();
    ui_context.reset();
    mesh_renderer.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
}
