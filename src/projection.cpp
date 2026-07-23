#include "projection.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

glm::mat4 build_projection_matrix(
    const ViewportFit& fit,
    const ProjectionControls& controls)
{
    if (controls.use_perspective == 0) {
        return glm::ortho(
            0.0F,
            static_cast<float>(fit.viewport_width),
            0.0F,
            static_cast<float>(fit.viewport_height),
            -10000.0F,
            10000.0F);
    }

    const float aspect = static_cast<float>(fit.viewport_width)
        / static_cast<float>(std::max(fit.viewport_height, 1));
    const float field_of_view = std::clamp(
        controls.fov_degrees,
        10.0F,
        120.0F);
    const float near_plane = std::max(controls.near_plane, 0.01F);
    const float far_plane = std::max(
        controls.far_plane,
        near_plane + 1.0F);
    const glm::mat4 center_viewport = glm::translate(
        glm::mat4(1.0F),
        glm::vec3(
            -static_cast<float>(fit.viewport_width) * 0.5F,
            -static_cast<float>(fit.viewport_height) * 0.5F,
            0.0F));
    return glm::perspective(
               glm::radians(field_of_view),
               aspect,
               near_plane,
               far_plane)
        * center_viewport;
}
