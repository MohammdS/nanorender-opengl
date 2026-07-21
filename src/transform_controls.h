#pragma once

#include <glm/vec3.hpp>

struct TransformControls {
    glm::vec3 local_translation {0.0F};
    glm::vec3 local_rotation {0.0F};
    glm::vec3 local_scale {1.0F};
    glm::vec3 world_translation {0.0F};
    glm::vec3 world_rotation {0.0F};
    glm::vec3 world_scale {1.0F};
};
