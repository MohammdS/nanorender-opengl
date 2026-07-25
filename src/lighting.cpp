#include "lighting.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <cmath>
#include <limits>

namespace {

glm::vec3 normalize_or_zero(const glm::vec3& value)
{
    const float length = glm::length(value);
    if (length <= std::numeric_limits<float>::epsilon()) {
        return glm::vec3(0.0F);
    }
    return value / length;
}

} // namespace

glm::vec3 calculate_ambient_lighting(
    const PointLight& light,
    const Material& material)
{
    return glm::clamp(
        light.ambient * material.ambient,
        glm::vec3(0.0F),
        glm::vec3(1.0F));
}

glm::vec3 calculate_flat_diffuse_lighting(
    const PointLight& light,
    const Material& material,
    const glm::vec3& surface_position,
    const glm::vec3& surface_normal,
    float* diffuse_factor)
{
    const glm::vec3 light_direction =
        normalize_or_zero(light.position - surface_position);
    const float lambert = glm::max(
        glm::dot(normalize_or_zero(surface_normal), light_direction),
        0.0F);
    if (diffuse_factor != nullptr) {
        *diffuse_factor = lambert;
    }

    const glm::vec3 ambient =
        calculate_ambient_lighting(light, material);
    const glm::vec3 diffuse =
        light.diffuse * material.diffuse * lambert;
    return glm::clamp(
        ambient + diffuse,
        glm::vec3(0.0F),
        glm::vec3(1.0F));
}

glm::vec3 calculate_reflection_vector(
    const glm::vec3& incident,
    const glm::vec3& surface_normal)
{
    const glm::vec3 normal = normalize_or_zero(surface_normal);
    const glm::vec3 incoming = normalize_or_zero(incident);
    return normalize_or_zero(
        incoming - 2.0F * glm::dot(normal, incoming) * normal);
}

glm::vec3 calculate_phong_lighting(
    const PointLight& light,
    const Material& material,
    const glm::vec3& surface_position,
    const glm::vec3& surface_normal,
    const glm::vec3& view_position,
    float* diffuse_factor,
    float* specular_factor)
{
    const glm::vec3 normal = normalize_or_zero(surface_normal);
    const glm::vec3 light_direction =
        normalize_or_zero(light.position - surface_position);
    const float lambert =
        glm::max(glm::dot(normal, light_direction), 0.0F);

    const glm::vec3 reflection =
        calculate_reflection_vector(-light_direction, normal);
    const glm::vec3 view_direction =
        normalize_or_zero(view_position - surface_position);
    const float reflection_alignment =
        glm::max(glm::dot(reflection, view_direction), 0.0F);
    const float shininess = glm::max(material.shininess, 1.0F);
    const float specular = lambert > 0.0F
        ? std::pow(reflection_alignment, shininess)
        : 0.0F;

    if (diffuse_factor != nullptr) {
        *diffuse_factor = lambert;
    }
    if (specular_factor != nullptr) {
        *specular_factor = specular;
    }

    const glm::vec3 ambient =
        calculate_ambient_lighting(light, material);
    const glm::vec3 diffuse =
        light.diffuse * material.diffuse * lambert;
    const glm::vec3 specular_color =
        light.specular * material.specular * specular;
    return glm::clamp(
        ambient + diffuse + specular_color,
        glm::vec3(0.0F),
        glm::vec3(1.0F));
}
