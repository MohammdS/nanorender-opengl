#include "transform_controls.h"

#include "mesh.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

glm::mat4 compose_trs_matrix(
    const glm::vec3& translation,
    const glm::vec3& rotation_degrees,
    const glm::vec3& scale)
{
    glm::mat4 matrix = glm::translate(glm::mat4(1.0F), translation);
    matrix = glm::rotate(
        matrix,
        glm::radians(rotation_degrees.z),
        glm::vec3(0.0F, 0.0F, 1.0F));
    matrix = glm::rotate(
        matrix,
        glm::radians(rotation_degrees.y),
        glm::vec3(0.0F, 1.0F, 0.0F));
    matrix = glm::rotate(
        matrix,
        glm::radians(rotation_degrees.x),
        glm::vec3(1.0F, 0.0F, 0.0F));
    return glm::scale(matrix, scale);
}

} // namespace

glm::mat4 build_local_transform_matrix(
    const TransformControls& controls,
    const ViewportFit& fit)
{
    const glm::vec3 pivot = apply_viewport_fit(fit.center, fit);
    glm::mat4 local_core(1.0F);
    local_core = glm::rotate(
        local_core,
        glm::radians(controls.local_rotation.z),
        glm::vec3(0.0F, 0.0F, 1.0F));
    local_core = glm::rotate(
        local_core,
        glm::radians(controls.local_rotation.y),
        glm::vec3(0.0F, 1.0F, 0.0F));
    local_core = glm::rotate(
        local_core,
        glm::radians(controls.local_rotation.x),
        glm::vec3(1.0F, 0.0F, 0.0F));
    local_core = glm::translate(local_core, controls.local_translation);
    local_core = glm::scale(local_core, controls.local_scale);
    return glm::translate(glm::mat4(1.0F), pivot) * local_core
        * glm::translate(glm::mat4(1.0F), -pivot);
}

glm::mat4 build_world_transform_matrix(const TransformControls& controls)
{
    return compose_trs_matrix(
        controls.world_translation,
        controls.world_rotation,
        controls.world_scale);
}

TransformControls make_local_then_world_preset()
{
    TransformControls controls;
    controls.local_translation = glm::vec3(100.0F, 0.0F, 0.0F);
    controls.world_rotation = glm::vec3(0.0F, 0.0F, 15.0F);
    return controls;
}

TransformControls make_world_then_local_preset()
{
    TransformControls controls;
    controls.world_translation = glm::vec3(100.0F, 0.0F, 0.0F);
    controls.local_rotation = glm::vec3(0.0F, 0.0F, 15.0F);
    return controls;
}

TransformControls make_hw3_task1_debug_preset()
{
    TransformControls controls;
    controls.local_rotation = glm::vec3(25.0F, -25.0F, 0.0F);
    return controls;
}

TransformControls make_hw4_task1_bounds_preset()
{
    TransformControls controls;
    controls.local_rotation = glm::vec3(40.0F, 50.0F, 30.0F);
    return controls;
}

glm::vec3& selected_transform_value(
    TransformControls& controls,
    const KeyboardTransformState& state)
{
    if (state.space == KeyboardTransformSpace::local) {
        if (state.mode == KeyboardTransformMode::translation) {
            return controls.local_translation;
        }
        if (state.mode == KeyboardTransformMode::rotation) {
            return controls.local_rotation;
        }
        return controls.local_scale;
    }

    if (state.mode == KeyboardTransformMode::translation) {
        return controls.world_translation;
    }
    if (state.mode == KeyboardTransformMode::rotation) {
        return controls.world_rotation;
    }
    return controls.world_scale;
}

const glm::vec3& selected_transform_value(
    const TransformControls& controls,
    const KeyboardTransformState& state)
{
    if (state.space == KeyboardTransformSpace::local) {
        if (state.mode == KeyboardTransformMode::translation) {
            return controls.local_translation;
        }
        if (state.mode == KeyboardTransformMode::rotation) {
            return controls.local_rotation;
        }
        return controls.local_scale;
    }

    if (state.mode == KeyboardTransformMode::translation) {
        return controls.world_translation;
    }
    if (state.mode == KeyboardTransformMode::rotation) {
        return controls.world_rotation;
    }
    return controls.world_scale;
}

void apply_keyboard_transform_step(
    TransformControls& controls,
    const KeyboardTransformState& state,
    const glm::vec3& direction)
{
    glm::vec3& value = selected_transform_value(controls, state);
    float step = 6.0F;
    if (state.mode == KeyboardTransformMode::rotation) {
        step = 1.5F;
    } else if (state.mode == KeyboardTransformMode::scale) {
        step = 0.02F;
    }
    value += direction * step;

    if (state.mode == KeyboardTransformMode::scale) {
        value = glm::clamp(value, glm::vec3(0.2F), glm::vec3(3.0F));
    }
}

const char* keyboard_transform_mode_name(KeyboardTransformMode mode)
{
    switch (mode) {
    case KeyboardTransformMode::translation:
        return "Translation";
    case KeyboardTransformMode::rotation:
        return "Rotation";
    case KeyboardTransformMode::scale:
        return "Scale";
    }
    return "Unknown";
}

const char* keyboard_transform_space_name(KeyboardTransformSpace space)
{
    return space == KeyboardTransformSpace::local ? "Local" : "World";
}
