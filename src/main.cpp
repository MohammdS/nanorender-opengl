#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "debug_renderer.h"
#include "gpu_ui_renderer.h"
#include "mesh.h"
#include "mesh_renderer.h"
#include "transform_controls.h"

#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
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
    hw2_task4,
    hw2_task5,
    hw2_task6,
    hw3_task1,
    hw3_task2,
    hw3_task3,
    hw3_task4,
};

enum class StartupPreset {
    none,
    local_then_world,
    world_then_local,
    hw3_task1_debug,
    hw3_task2_camera,
    hw3_task3_projection,
    hw3_task4_normals,
};

struct CommandLineOptions {
    ValidationMode validation = ValidationMode::none;
    bool valid = true;
    StartupPreset preset = StartupPreset::none;
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
        if (feature == "hw2-task4") {
            return {.validation = ValidationMode::hw2_task4, .valid = true};
        }
        if (feature == "hw2-task5") {
            return {.validation = ValidationMode::hw2_task5, .valid = true};
        }
        if (feature == "hw2-task6") {
            return {.validation = ValidationMode::hw2_task6, .valid = true};
        }
        if (feature == "hw3-task1") {
            return {.validation = ValidationMode::hw3_task1, .valid = true};
        }
        if (feature == "hw3-task2") {
            return {.validation = ValidationMode::hw3_task2, .valid = true};
        }
        if (feature == "hw3-task3") {
            return {.validation = ValidationMode::hw3_task3, .valid = true};
        }
        if (feature == "hw3-task4") {
            return {.validation = ValidationMode::hw3_task4, .valid = true};
        }
    }

    if (argc == 3 && std::string_view(argv[1]) == "--preset") {
        const std::string_view preset = argv[2];
        if (preset == "hw2-task5-local-world") {
            return {
                .validation = ValidationMode::none,
                .valid = true,
                .preset = StartupPreset::local_then_world,
            };
        }
        if (preset == "hw2-task5-world-local") {
            return {
                .validation = ValidationMode::none,
                .valid = true,
                .preset = StartupPreset::world_then_local,
            };
        }
        if (preset == "hw3-task1-debug") {
            return {
                .validation = ValidationMode::none,
                .valid = true,
                .preset = StartupPreset::hw3_task1_debug,
            };
        }
        if (preset == "hw3-task2-camera") {
            return {
                .validation = ValidationMode::none,
                .valid = true,
                .preset = StartupPreset::hw3_task2_camera,
            };
        }
        if (preset == "hw3-task3-projection") {
            return {
                .validation = ValidationMode::none,
                .valid = true,
                .preset = StartupPreset::hw3_task3_projection,
            };
        }
        if (preset == "hw3-task4-normals") {
            return {
                .validation = ValidationMode::none,
                .valid = true,
                .preset = StartupPreset::hw3_task4_normals,
            };
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

void update_keyboard_transform_input(
    GLFWwindow* window,
    TransformControls& controls,
    KeyboardTransformState& state)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        state.mode = KeyboardTransformMode::translation;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        state.mode = KeyboardTransformMode::rotation;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        state.mode = KeyboardTransformMode::scale;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        state.space = KeyboardTransformSpace::local;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        state.space = KeyboardTransformSpace::world;
    }

    glm::vec3 direction(0.0F);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        direction.x -= 1.0F;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        direction.x += 1.0F;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        direction.y += 1.0F;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        direction.y -= 1.0F;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS) {
        direction.z += 1.0F;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS) {
        direction.z -= 1.0F;
    }

    if (direction.x != 0.0F || direction.y != 0.0F
        || direction.z != 0.0F) {
        apply_keyboard_transform_step(controls, state, direction);
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
            && std::filesystem::is_regular_file(candidate / "wireframe.frag")
            && std::filesystem::is_regular_file(candidate / "debug.vert")
            && std::filesystem::is_regular_file(candidate / "debug.frag")) {
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
}

void draw_slider_control(
    mu_Context& context,
    const char* axis,
    float& value,
    float minimum,
    float maximum)
{
    int widths[] {32, -1};
    mu_layout_row(&context, 2, widths, 0);
    mu_label(&context, axis);
    mu_slider(&context, &value, minimum, maximum);
}

void draw_vec3_controls(
    mu_Context& context,
    const char* label,
    glm::vec3& value,
    float minimum,
    float maximum)
{
    int full_width[] {-1};
    mu_layout_row(&context, 1, full_width, 0);
    mu_label(&context, label);
    draw_slider_control(context, "X", value.x, minimum, maximum);
    draw_slider_control(context, "Y", value.y, minimum, maximum);
    draw_slider_control(context, "Z", value.z, minimum, maximum);
}

void draw_compact_vec3_controls(
    mu_Context& context,
    const char* label,
    glm::vec3& value,
    float minimum,
    float maximum)
{
    int full_width[] {-1};
    mu_layout_row(&context, 1, full_width, 0);
    mu_label(&context, label);
    int widths[] {125, 125, -1};
    mu_layout_row(&context, 3, widths, 0);
    mu_slider(&context, &value.x, minimum, maximum);
    mu_slider(&context, &value.y, minimum, maximum);
    mu_slider(&context, &value.z, minimum, maximum);
}

void draw_named_slider(
    mu_Context& context,
    const char* label,
    float& value,
    float minimum,
    float maximum)
{
    int widths[] {110, -1};
    mu_layout_row(&context, 2, widths, 0);
    mu_label(&context, label);
    mu_slider(&context, &value, minimum, maximum);
}

void build_transform_controls_window(
    mu_Context& context,
    TransformControls& controls,
    const KeyboardTransformState& keyboard_state,
    bool& initialized,
    bool initially_open)
{
    mu_Container* container =
        mu_get_container(&context, "HW2 Transform Controls");
    if (!initialized) {
        container->open = initially_open ? 1 : 0;
        initialized = true;
    }

    if (mu_begin_window(
            &context,
            "HW2 Transform Controls",
            mu_rect(490, 24, 765, 640))) {
        int full_width[] {-1};
        mu_layout_row(&context, 1, full_width, 0);
        mu_label(&context, "GPU matrix order: World * Local * fitted vertex");
        mu_label(&context, "Keys: Q/W/E = Translate/Rotate/Scale");
        mu_label(&context, "Z/X = Local/World | Arrows = X/Y | PgUp/PgDn = Z");

        const glm::vec3& selected =
            selected_transform_value(controls, keyboard_state);
        char keyboard_status[160] {};
        std::snprintf(
            keyboard_status,
            sizeof(keyboard_status),
            "Keyboard: %s %s | X %.2f  Y %.2f  Z %.2f",
            keyboard_transform_space_name(keyboard_state.space),
            keyboard_transform_mode_name(keyboard_state.mode),
            selected.x,
            selected.y,
            selected.z);
        mu_label(&context, keyboard_status);

        int columns[] {350, -1};
        mu_layout_row(&context, 2, columns, 410);

        mu_begin_panel(&context, "Local Transform Panel");
        mu_layout_row(&context, 1, full_width, 0);
        mu_label(&context, "Local frame");
        draw_vec3_controls(
            context,
            "Translation (-600 to 600)",
            controls.local_translation,
            -600.0F,
            600.0F);
        draw_vec3_controls(
            context,
            "Rotation degrees (-180 to 180)",
            controls.local_rotation,
            -180.0F,
            180.0F);
        draw_vec3_controls(
            context,
            "Scale (0.2 to 3.0)",
            controls.local_scale,
            0.2F,
            3.0F);
        mu_end_panel(&context);

        mu_begin_panel(&context, "World Transform Panel");
        mu_layout_row(&context, 1, full_width, 0);
        mu_label(&context, "World frame");
        draw_vec3_controls(
            context,
            "Translation (-600 to 600)",
            controls.world_translation,
            -600.0F,
            600.0F);
        draw_vec3_controls(
            context,
            "Rotation degrees (-180 to 180)",
            controls.world_rotation,
            -180.0F,
            180.0F);
        draw_vec3_controls(
            context,
            "Scale (0.2 to 3.0)",
            controls.world_scale,
            0.2F,
            3.0F);
        mu_end_panel(&context);

        int preset_widths[] {-1, -1};
        mu_layout_row(&context, 2, preset_widths, 0);
        if (mu_button(&context, "Local translation -> World rotation")) {
            controls = make_local_then_world_preset();
        }
        if (mu_button(&context, "World translation -> Local rotation")) {
            controls = make_world_then_local_preset();
        }

        mu_layout_row(&context, 1, full_width, 0);
        if (mu_button(&context, "Reset Local and World Transforms")) {
            controls = TransformControls {};
        }
        mu_end_window(&context);
    }
}

void build_hw3_debug_window(
    mu_Context& context,
    DebugVisualControls& controls,
    CameraControls& camera,
    ProjectionControls& projection,
    bool& initialized)
{
    mu_Container* container = mu_get_container(&context, "HW3 Camera / Debug");
    if (!initialized) {
        container->open = 1;
        initialized = true;
    }

    if (mu_begin_window(
            &context,
            "HW3 Camera / Debug",
            mu_rect(24, 286, 430, 424))) {
        int full_width[] {-1};
        mu_layout_row(&context, 1, full_width, 0);
        mu_label(&context, "Task 1 GPU debug visuals");
        mu_checkbox(&context, "Show Local Axes", &controls.show_local_axes);
        mu_checkbox(&context, "Show World Axes", &controls.show_world_axes);
        mu_checkbox(
            &context,
            "Show Bounding Box",
            &controls.show_bounding_box);
        mu_label(&context, "Task 2 virtual camera (inverse view matrix)");
        draw_compact_vec3_controls(
            context,
            "Position X / Y / Z (-2000 to 2000)",
            camera.position,
            -2000.0F,
            2000.0F);
        draw_compact_vec3_controls(
            context,
            "Rotation X / Y / Z (-180 to 180)",
            camera.rotation,
            -180.0F,
            180.0F);
        mu_label(&context, "Task 3 projection");
        int action_widths[] {190, -1};
        mu_layout_row(&context, 2, action_widths, 0);
        if (mu_button(&context, "Reset Camera")) {
            camera = CameraControls {};
        }
        if (mu_button(
                &context,
                projection.use_perspective != 0
                    ? "Projection: Perspective"
                    : "Projection: Orthographic")) {
            projection.use_perspective =
                projection.use_perspective == 0 ? 1 : 0;
        }
        draw_named_slider(
            context,
            "Field of view",
            projection.fov_degrees,
            10.0F,
            120.0F);
        draw_named_slider(
            context,
            "Near plane",
            projection.near_plane,
            0.1F,
            100.0F);
        draw_named_slider(
            context,
            "Far plane",
            projection.far_plane,
            1000.0F,
            10000.0F);
        mu_layout_row(&context, 1, full_width, 0);
        mu_label(&context, "Task 4 GPU normal visuals");
        int normal_widths[] {190, -1};
        mu_layout_row(&context, 2, normal_widths, 0);
        mu_checkbox(
            &context,
            "Show Face Normals",
            &controls.show_face_normals);
        mu_checkbox(
            &context,
            "Show Vertex Normals",
            &controls.show_vertex_normals);
        mu_end_window(&context);
    }
}

bool ui_sample_differs_from_background(
    int sample_x,
    int sample_y_from_top,
    int width,
    int height)
{
    if (sample_x < 0 || sample_x >= width || sample_y_from_top < 0
        || sample_y_from_top >= height) {
        return false;
    }

    std::array<unsigned char, 4> pixel {};
    glReadPixels(
        sample_x,
        height - 1 - sample_y_from_top,
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

bool popup_sample_differs_from_background(int width, int height)
{
    return ui_sample_differs_from_background(30, 60, width, height);
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

struct WireframeSample {
    std::size_t pixel_count = 0;
    float center_x = 0.0F;
    float center_y = 0.0F;
    int min_x = std::numeric_limits<int>::max();
    int max_x = -1;
};

WireframeSample read_wireframe_sample(int width, int height)
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

    WireframeSample sample;
    double x_sum = 0.0;
    double y_sum = 0.0;
    for (std::size_t offset = 0; offset < pixels.size(); offset += 3) {
        const unsigned char red = pixels[offset];
        const unsigned char green = pixels[offset + 1];
        const unsigned char blue = pixels[offset + 2];
        if (red >= 35 && red <= 70 && green >= 185 && blue >= 230) {
            const std::size_t pixel_index = offset / 3;
            const int x = static_cast<int>(pixel_index % width);
            x_sum += static_cast<double>(x);
            y_sum += static_cast<double>(pixel_index / width);
            sample.min_x = std::min(sample.min_x, x);
            sample.max_x = std::max(sample.max_x, x);
            ++sample.pixel_count;
        }
    }

    if (sample.pixel_count > 0) {
        sample.center_x = static_cast<float>(
            x_sum / static_cast<double>(sample.pixel_count));
        sample.center_y = static_cast<float>(
            y_sum / static_cast<double>(sample.pixel_count));
    }
    return sample;
}

bool validate_hw2_task3(
    const Mesh& mesh,
    const MeshRenderer& renderer,
    int width,
    int height)
{
    const WireframeSample sample = read_wireframe_sample(width, height);

    const std::size_t expected_indices = mesh.faces.size() * 6;
    const bool gpu_draw = sample.pixel_count >= 100;
    std::cout << "HW2 Task 3 indexed wireframe: vertices="
              << mesh.vertices.size() << " faces=" << mesh.faces.size()
              << " line_indices=" << renderer.edge_index_count()
              << " wireframe_pixels=" << sample.pixel_count
              << " gpu_draw=" << (gpu_draw ? "yes" : "no") << '\n';
    return renderer.edge_index_count() == expected_indices && gpu_draw
        && glGetError() == GL_NO_ERROR;
}

bool vec3_nearly_equal(const glm::vec3& value, const glm::vec3& expected)
{
    return nearly_equal(value.x, expected.x)
        && nearly_equal(value.y, expected.y)
        && nearly_equal(value.z, expected.z);
}

bool validate_hw2_task4(
    const TransformControls& controls,
    int width,
    int height)
{
    const bool defaults_are_valid =
        vec3_nearly_equal(controls.local_translation, glm::vec3(0.0F))
        && vec3_nearly_equal(controls.local_rotation, glm::vec3(0.0F))
        && vec3_nearly_equal(controls.local_scale, glm::vec3(1.0F))
        && vec3_nearly_equal(controls.world_translation, glm::vec3(0.0F))
        && vec3_nearly_equal(controls.world_rotation, glm::vec3(0.0F))
        && vec3_nearly_equal(controls.world_scale, glm::vec3(1.0F));
    const bool panel_rendered =
        ui_sample_differs_from_background(510, 60, width, height);

    std::cout << "HW2 Task 4 transform controls: values=18 local=yes "
                 "world=yes defaults="
              << (defaults_are_valid ? "yes" : "no")
              << " panel_rendered=" << (panel_rendered ? "yes" : "no")
              << '\n';
    return defaults_are_valid && panel_rendered
        && glGetError() == GL_NO_ERROR;
}

WireframeSample render_and_sample_transform(
    const MeshRenderer& renderer,
    const ViewportFit& fit,
    const TransformControls& controls)
{
    glClear(GL_COLOR_BUFFER_BIT);
    renderer.render(fit, controls);
    return read_wireframe_sample(fit.viewport_width, fit.viewport_height);
}

bool validate_hw2_task5(
    const MeshRenderer& renderer,
    const ViewportFit& fit)
{
    const WireframeSample local_then_world = render_and_sample_transform(
        renderer,
        fit,
        make_local_then_world_preset());
    const WireframeSample world_then_local = render_and_sample_transform(
        renderer,
        fit,
        make_world_then_local_preset());
    const float separation = std::hypot(
        local_then_world.center_x - world_then_local.center_x,
        local_then_world.center_y - world_then_local.center_y);
    const bool both_rendered = local_then_world.pixel_count >= 100
        && world_then_local.pixel_count >= 100;
    const bool order_is_visible = separation >= 50.0F;

    std::cout << std::fixed << std::setprecision(1)
              << "HW2 Task 5 GPU transforms: local_world_center=("
              << local_then_world.center_x << ',' << local_then_world.center_y
              << ") world_local_center=(" << world_then_local.center_x << ','
              << world_then_local.center_y << ") separation=" << separation
              << " gpu_draw=" << (both_rendered ? "yes" : "no")
              << " order_visible=" << (order_is_visible ? "yes" : "no")
              << '\n'
              << std::defaultfloat;
    return both_rendered && order_is_visible
        && glGetError() == GL_NO_ERROR;
}

bool validate_hw2_task6(
    const MeshRenderer& renderer,
    const ViewportFit& fit)
{
    TransformControls movement_controls;
    KeyboardTransformState movement_state;
    const WireframeSample original = render_and_sample_transform(
        renderer,
        fit,
        movement_controls);
    apply_keyboard_transform_step(
        movement_controls,
        movement_state,
        glm::vec3(1.0F, 0.0F, 0.0F));
    const WireframeSample moved = render_and_sample_transform(
        renderer,
        fit,
        movement_controls);
    const float screen_movement = std::hypot(
        moved.center_x - original.center_x,
        moved.center_y - original.center_y);

    TransformControls value_checks;
    KeyboardTransformState state;
    apply_keyboard_transform_step(
        value_checks,
        state,
        glm::vec3(1.0F, -1.0F, 1.0F));
    const bool translation_step = vec3_nearly_equal(
        value_checks.world_translation,
        glm::vec3(6.0F, -6.0F, 6.0F));

    state.mode = KeyboardTransformMode::rotation;
    state.space = KeyboardTransformSpace::local;
    apply_keyboard_transform_step(
        value_checks,
        state,
        glm::vec3(-1.0F, 1.0F, 0.0F));
    const bool rotation_step = vec3_nearly_equal(
        value_checks.local_rotation,
        glm::vec3(-1.5F, 1.5F, 0.0F));

    state.mode = KeyboardTransformMode::scale;
    state.space = KeyboardTransformSpace::world;
    value_checks.world_scale = glm::vec3(2.99F);
    apply_keyboard_transform_step(
        value_checks,
        state,
        glm::vec3(1.0F));
    const bool maximum_scale_clamped = vec3_nearly_equal(
        value_checks.world_scale,
        glm::vec3(3.0F));
    value_checks.world_scale = glm::vec3(0.21F);
    apply_keyboard_transform_step(
        value_checks,
        state,
        glm::vec3(-1.0F));
    const bool scale_clamped = maximum_scale_clamped
        && vec3_nearly_equal(
            value_checks.world_scale,
            glm::vec3(0.2F));
    const bool gpu_updated = original.pixel_count >= 100
        && moved.pixel_count >= 100 && screen_movement >= 4.0F;

    std::cout << std::fixed << std::setprecision(1)
              << "HW2 Task 6 keyboard input: translation_step="
              << (translation_step ? "yes" : "no")
              << " rotation_step=" << (rotation_step ? "yes" : "no")
              << " scale_clamped=" << (scale_clamped ? "yes" : "no")
              << " screen_movement=" << screen_movement
              << " gpu_updated=" << (gpu_updated ? "yes" : "no") << '\n'
              << std::defaultfloat;
    return translation_step && rotation_step && scale_clamped && gpu_updated
        && glGetError() == GL_NO_ERROR;
}

std::size_t count_debug_pixels(int width, int height)
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

    std::size_t count = 0;
    for (std::size_t offset = 0; offset < pixels.size(); offset += 3) {
        const int red = pixels[offset];
        const int green = pixels[offset + 1];
        const int blue = pixels[offset + 2];
        const bool yellow = red >= 200 && green >= 130 && blue <= 90;
        const bool red_axis = red >= 130 && green <= 100 && blue <= 100;
        const bool green_axis = green >= 120 && red <= 100 && blue <= 130;
        const bool blue_axis = blue >= 130 && red <= 130 && green <= 160;
        if (yellow || red_axis || green_axis || blue_axis) {
            ++count;
        }
    }
    return count;
}

bool validate_hw3_task1(
    const MeshRenderer& mesh_renderer,
    const DebugRenderer& debug_renderer,
    const ViewportFit& fit)
{
    const TransformControls transforms = make_hw3_task1_debug_preset();
    DebugVisualControls controls;
    controls.show_local_axes = 1;
    controls.show_world_axes = 1;
    controls.show_bounding_box = 1;

    glClear(GL_COLOR_BUFFER_BIT);
    mesh_renderer.render(fit, transforms);
    const DebugLineCounts counts = debug_renderer.render(
        fit,
        transforms,
        controls);
    const std::size_t debug_pixels = count_debug_pixels(
        fit.viewport_width,
        fit.viewport_height);
    const bool expected_counts = counts.local_axes == 3
        && counts.world_axes == 3 && counts.bounding_box_edges == 12
        && counts.total() == 18;
    const std::size_t rendered_faces = mesh_renderer.edge_index_count() / 6;
    const bool gpu_rendered = debug_pixels >= 100;

    std::cout << "HW3 Task 1 debug visuals: local_axes="
              << counts.local_axes << " world_axes=" << counts.world_axes
              << " bounding_box_edges=" << counts.bounding_box_edges
              << " total_debug_lines=" << counts.total()
              << " rendered_faces=" << rendered_faces
              << " debug_pixels=" << debug_pixels
              << " gpu_rendered=" << (gpu_rendered ? "yes" : "no")
              << '\n';
    return expected_counts && rendered_faces == 4 && gpu_rendered
        && glGetError() == GL_NO_ERROR;
}

WireframeSample render_and_sample_camera(
    const MeshRenderer& renderer,
    const ViewportFit& fit,
    const CameraControls& camera)
{
    glClear(GL_COLOR_BUFFER_BIT);
    renderer.render(fit, TransformControls {}, camera);
    return read_wireframe_sample(fit.viewport_width, fit.viewport_height);
}

bool validate_hw3_task2(
    const MeshRenderer& renderer,
    const ViewportFit& fit)
{
    const CameraControls default_camera;
    CameraControls left_camera;
    left_camera.position.x = -120.0F;

    const WireframeSample default_sample = render_and_sample_camera(
        renderer,
        fit,
        default_camera);
    const WireframeSample left_sample = render_and_sample_camera(
        renderer,
        fit,
        left_camera);
    const float shift = left_sample.center_x - default_sample.center_x;
    const bool both_rendered = default_sample.pixel_count >= 100
        && left_sample.pixel_count >= 100;
    const bool inverse_view_confirmed = nearly_equal(shift, 120.0F, 1.0F);

    std::cout << std::fixed << std::setprecision(1)
              << "HW3 Task 2 camera: default_center_x="
              << default_sample.center_x << " camera_left_center_x="
              << left_sample.center_x << " shift=" << shift
              << " inverse_view="
              << (inverse_view_confirmed ? "yes" : "no") << '\n'
              << std::defaultfloat;
    return both_rendered && inverse_view_confirmed
        && glGetError() == GL_NO_ERROR;
}

WireframeSample render_and_sample_projection(
    const MeshRenderer& renderer,
    const ViewportFit& fit,
    const TransformControls& transforms,
    const CameraControls& camera,
    const ProjectionControls& projection)
{
    glClear(GL_COLOR_BUFFER_BIT);
    renderer.render(fit, transforms, camera, projection);
    return read_wireframe_sample(fit.viewport_width, fit.viewport_height);
}

bool validate_hw3_task3(
    const MeshRenderer& renderer,
    const ViewportFit& fit)
{
    const TransformControls transforms = make_hw3_task1_debug_preset();
    const CameraControls camera;
    const ProjectionControls orthographic;
    ProjectionControls perspective;
    perspective.use_perspective = 1;

    const WireframeSample orthographic_sample =
        render_and_sample_projection(
            renderer,
            fit,
            transforms,
            camera,
            orthographic);
    const WireframeSample perspective_sample =
        render_and_sample_projection(
            renderer,
            fit,
            transforms,
            camera,
            perspective);
    const int orthographic_width =
        orthographic_sample.max_x - orthographic_sample.min_x + 1;
    const int perspective_width =
        perspective_sample.max_x - perspective_sample.min_x + 1;
    const bool both_rendered = orthographic_sample.pixel_count >= 100
        && perspective_sample.pixel_count >= 100;
    const bool perspective_is_visible =
        perspective_width + 20 <= orthographic_width;

    std::cout << std::fixed << std::setprecision(1)
              << "HW3 Task 3 projection: orthographic_width="
              << orthographic_width << " perspective_width="
              << perspective_width << " fov=" << perspective.fov_degrees
              << " near=" << perspective.near_plane
              << " far=" << perspective.far_plane
              << " perspective_visible="
              << (perspective_is_visible ? "yes" : "no") << '\n'
              << std::defaultfloat;
    return both_rendered && perspective_is_visible
        && glGetError() == GL_NO_ERROR;
}

std::size_t count_normal_pixels(int width, int height)
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

    std::size_t count = 0;
    for (std::size_t offset = 0; offset < pixels.size(); offset += 3) {
        const int red = pixels[offset];
        const int green = pixels[offset + 1];
        const int blue = pixels[offset + 2];
        const bool face_normal =
            red >= 200 && green <= 110 && blue >= 130;
        const bool vertex_normal =
            red >= 200 && green >= 170 && blue <= 110;
        if (face_normal || vertex_normal) {
            ++count;
        }
    }
    return count;
}

std::size_t count_unit_normals(const std::vector<glm::vec3>& normals)
{
    return static_cast<std::size_t>(std::count_if(
        normals.begin(),
        normals.end(),
        [](const glm::vec3& normal) {
            return nearly_equal(glm::length(normal), 1.0F, 0.001F);
        }));
}

bool validate_hw3_task4(
    const Mesh& mesh,
    const MeshNormals& normals,
    const DebugRenderer& renderer,
    const ViewportFit& fit)
{
    DebugVisualControls controls;
    controls.show_face_normals = 1;
    controls.show_vertex_normals = 1;
    const TransformControls transforms = make_hw3_task1_debug_preset();

    glClear(GL_COLOR_BUFFER_BIT);
    const DebugLineCounts drawn = renderer.render(
        fit,
        transforms,
        controls,
        CameraControls {},
        ProjectionControls {},
        &mesh,
        &normals);
    const std::size_t unit_face_normals =
        count_unit_normals(normals.face_normals);
    const std::size_t unit_vertex_normals =
        count_unit_normals(normals.vertex_normals);
    const std::size_t normal_pixels = count_normal_pixels(
        fit.viewport_width,
        fit.viewport_height);
    const bool counts_are_valid =
        normals.face_normals.size() == mesh.faces.size()
        && normals.vertex_normals.size() == mesh.vertices.size()
        && drawn.face_normals == mesh.faces.size()
        && drawn.vertex_normals == mesh.vertices.size();
    const bool normals_are_unit =
        unit_face_normals == normals.face_normals.size()
        && unit_vertex_normals == normals.vertex_normals.size();
    const bool gpu_rendered = normal_pixels >= 100;

    std::cout << "HW3 Task 4 normals: face_normals="
              << normals.face_normals.size()
              << " unit_face_normals=" << unit_face_normals
              << " vertex_normals=" << normals.vertex_normals.size()
              << " unit_vertex_normals=" << unit_vertex_normals
              << " drawn_face_lines=" << drawn.face_normals
              << " drawn_vertex_lines=" << drawn.vertex_normals
              << " normal_pixels=" << normal_pixels << '\n';
    return counts_are_valid && normals_are_unit && gpu_rendered
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
                     "hw2-task3|hw2-task4|hw2-task5|hw2-task6|"
                     "hw3-task1|hw3-task2|hw3-task3|hw3-task4] or "
                     "[--preset hw2-task5-local-world|"
                     "hw2-task5-world-local|hw3-task1-debug|"
                     "hw3-task2-camera|hw3-task3-projection|"
                     "hw3-task4-normals]\n";
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
    MeshNormals mesh_normals;
    std::filesystem::path mesh_path;
    ViewportFit viewport_fit;
    try {
        mesh_path = find_model_path(argv[0]);
        mesh = load_obj_mesh(mesh_path);
        mesh_normals = calculate_mesh_normals(mesh);
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
    std::unique_ptr<DebugRenderer> debug_renderer;
    if (options.validation == ValidationMode::none
        || options.validation == ValidationMode::hw2_task3
        || options.validation == ValidationMode::hw2_task4
        || options.validation == ValidationMode::hw2_task5
        || options.validation == ValidationMode::hw2_task6
        || options.validation == ValidationMode::hw3_task1
        || options.validation == ValidationMode::hw3_task2
        || options.validation == ValidationMode::hw3_task3
        || options.validation == ValidationMode::hw3_task4) {
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
    if (options.validation == ValidationMode::none
        || options.validation == ValidationMode::hw3_task1
        || options.validation == ValidationMode::hw3_task4) {
        try {
            debug_renderer = std::make_unique<DebugRenderer>(
                find_shader_directory(argv[0]));
        } catch (const std::exception& exception) {
            std::cerr << "GPU debug initialization failed: "
                      << exception.what() << '\n';
            mesh_renderer.reset();
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
    TransformControls transform_controls;
    KeyboardTransformState keyboard_transform_state;
    DebugVisualControls debug_visuals;
    CameraControls camera;
    ProjectionControls projection;
    if (options.preset == StartupPreset::local_then_world) {
        transform_controls = make_local_then_world_preset();
    } else if (options.preset == StartupPreset::world_then_local) {
        transform_controls = make_world_then_local_preset();
    } else if (options.preset == StartupPreset::hw3_task1_debug) {
        transform_controls = make_hw3_task1_debug_preset();
        debug_visuals.show_local_axes = 1;
        debug_visuals.show_world_axes = 1;
        debug_visuals.show_bounding_box = 1;
    } else if (options.preset == StartupPreset::hw3_task2_camera) {
        camera.position.x = -120.0F;
    } else if (options.preset == StartupPreset::hw3_task3_projection) {
        transform_controls = make_hw3_task1_debug_preset();
        projection.use_perspective = 1;
    } else if (options.preset == StartupPreset::hw3_task4_normals) {
        transform_controls = make_hw3_task1_debug_preset();
        debug_visuals.show_face_normals = 1;
        debug_visuals.show_vertex_normals = 1;
    }
    bool popup_initialized = false;
    bool transform_controls_initialized = false;
    bool hw3_debug_initialized = false;
    int exit_code = EXIT_SUCCESS;
    while (glfwWindowShouldClose(window) != GLFW_TRUE) {
        process_input(window);
        if (options.validation == ValidationMode::none) {
            update_keyboard_transform_input(
                window,
                transform_controls,
                keyboard_transform_state);
        }

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
            mu_begin(ui_context.get());
            build_mesh_info_popup(
                *ui_context,
                mesh_path,
                mesh,
                viewport_fit,
                toggle_popup,
                popup_initialized);
            if (options.validation == ValidationMode::none
                || options.validation == ValidationMode::hw2_task4
                || options.validation == ValidationMode::hw2_task5
                || options.validation == ValidationMode::hw2_task6) {
                build_transform_controls_window(
                    *ui_context,
                    transform_controls,
                    keyboard_transform_state,
                    transform_controls_initialized,
                    options.preset == StartupPreset::none);
            }
            if (options.validation == ValidationMode::none
                || options.validation == ValidationMode::hw3_task1
                || options.validation == ValidationMode::hw3_task2
                || options.validation == ValidationMode::hw3_task3
                || options.validation == ValidationMode::hw3_task4) {
                build_hw3_debug_window(
                    *ui_context,
                    debug_visuals,
                    camera,
                    projection,
                    hw3_debug_initialized);
            }
            mu_end(ui_context.get());
        }

        glClearColor(
            clear_color[0],
            clear_color[1],
            clear_color[2],
            clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        if (mesh_renderer != nullptr) {
            mesh_renderer->render(
                viewport_fit,
                transform_controls,
                camera,
                projection);
        }
        if (debug_renderer != nullptr) {
            debug_renderer->render(
                viewport_fit,
                transform_controls,
                debug_visuals,
                camera,
                projection,
                &mesh,
                &mesh_normals);
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
            } else if (options.validation == ValidationMode::hw2_task3) {
                passed = validate_hw2_task3(
                    mesh,
                    *mesh_renderer,
                    framebuffer_width,
                    framebuffer_height);
                validation_name = "HW2 Task 3";
            } else if (options.validation == ValidationMode::hw2_task4) {
                passed = validate_hw2_task4(
                    transform_controls,
                    framebuffer_width,
                    framebuffer_height);
                validation_name = "HW2 Task 4";
            } else if (options.validation == ValidationMode::hw2_task5) {
                passed = validate_hw2_task5(*mesh_renderer, viewport_fit);
                validation_name = "HW2 Task 5";
            } else if (options.validation == ValidationMode::hw2_task6) {
                passed = validate_hw2_task6(*mesh_renderer, viewport_fit);
                validation_name = "HW2 Task 6";
            } else if (options.validation == ValidationMode::hw3_task1) {
                passed = validate_hw3_task1(
                    *mesh_renderer,
                    *debug_renderer,
                    viewport_fit);
                validation_name = "HW3 Task 1";
            } else if (options.validation == ValidationMode::hw3_task2) {
                passed = validate_hw3_task2(*mesh_renderer, viewport_fit);
                validation_name = "HW3 Task 2";
            } else if (options.validation == ValidationMode::hw3_task3) {
                passed = validate_hw3_task3(*mesh_renderer, viewport_fit);
                validation_name = "HW3 Task 3";
            } else {
                passed = validate_hw3_task4(
                    mesh,
                    mesh_normals,
                    *debug_renderer,
                    viewport_fit);
                validation_name = "HW3 Task 4";
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
    debug_renderer.reset();
    mesh_renderer.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
}
