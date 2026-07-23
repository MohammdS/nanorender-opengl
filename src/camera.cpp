#include "camera.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 build_camera_view_matrix(const CameraControls& camera)
{
    glm::mat4 camera_transform =
        glm::translate(glm::mat4(1.0F), camera.position);
    camera_transform = glm::rotate(
        camera_transform,
        glm::radians(camera.rotation.z),
        glm::vec3(0.0F, 0.0F, 1.0F));
    camera_transform = glm::rotate(
        camera_transform,
        glm::radians(camera.rotation.y),
        glm::vec3(0.0F, 1.0F, 0.0F));
    camera_transform = glm::rotate(
        camera_transform,
        glm::radians(camera.rotation.x),
        glm::vec3(1.0F, 0.0F, 0.0F));
    return glm::inverse(camera_transform);
}
