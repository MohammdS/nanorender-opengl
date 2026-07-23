#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct ViewportFit;

struct TransformControls {
    glm::vec3 local_translation {0.0F};
    glm::vec3 local_rotation {0.0F};
    glm::vec3 local_scale {1.0F};
    glm::vec3 world_translation {0.0F};
    glm::vec3 world_rotation {0.0F};
    glm::vec3 world_scale {1.0F};
};

enum class KeyboardTransformMode {
    translation,
    rotation,
    scale,
};

enum class KeyboardTransformSpace {
    local,
    world,
};

struct KeyboardTransformState {
    KeyboardTransformMode mode = KeyboardTransformMode::translation;
    KeyboardTransformSpace space = KeyboardTransformSpace::world;
};

glm::mat4 build_local_transform_matrix(
    const TransformControls& controls,
    const ViewportFit& fit);
glm::mat4 build_world_transform_matrix(const TransformControls& controls);

TransformControls make_local_then_world_preset();
TransformControls make_world_then_local_preset();
TransformControls make_hw3_task1_debug_preset();

glm::vec3& selected_transform_value(
    TransformControls& controls,
    const KeyboardTransformState& state);
const glm::vec3& selected_transform_value(
    const TransformControls& controls,
    const KeyboardTransformState& state);
void apply_keyboard_transform_step(
    TransformControls& controls,
    const KeyboardTransformState& state,
    const glm::vec3& direction);
const char* keyboard_transform_mode_name(KeyboardTransformMode mode);
const char* keyboard_transform_space_name(KeyboardTransformSpace space);
