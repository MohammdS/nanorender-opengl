#include "lighting.h"

#include <glm/common.hpp>

glm::vec3 calculate_ambient_lighting(
    const PointLight& light,
    const Material& material)
{
    return glm::clamp(
        light.ambient * material.ambient,
        glm::vec3(0.0F),
        glm::vec3(1.0F));
}
