#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct CameraControls {
    glm::vec3 position {0.0F, 0.0F, 1000.0F};
    glm::vec3 rotation {0.0F};
};

glm::mat4 build_camera_view_matrix(const CameraControls& camera);
