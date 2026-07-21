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

glm::mat4 build_local_transform_matrix(
    const TransformControls& controls,
    const ViewportFit& fit);
glm::mat4 build_world_transform_matrix(const TransformControls& controls);

TransformControls make_local_then_world_preset();
TransformControls make_world_then_local_preset();
