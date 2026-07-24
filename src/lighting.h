#pragma once

#include <glm/vec3.hpp>

struct PointLight {
    glm::vec3 position {400.0F, 400.0F, 800.0F};
    glm::vec3 ambient {0.6F};
    glm::vec3 diffuse {1.0F};
    glm::vec3 specular {1.0F};
};

struct Material {
    glm::vec3 ambient {0.25F, 0.65F, 0.9F};
    glm::vec3 diffuse {0.25F, 0.65F, 0.9F};
    glm::vec3 specular {1.0F};
};

glm::vec3 calculate_ambient_lighting(
    const PointLight& light,
    const Material& material);

glm::vec3 calculate_flat_diffuse_lighting(
    const PointLight& light,
    const Material& material,
    const glm::vec3& surface_position,
    const glm::vec3& surface_normal,
    float* diffuse_factor = nullptr);
