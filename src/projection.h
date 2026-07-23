#pragma once

#include "mesh.h"

#include <glm/mat4x4.hpp>

struct ProjectionControls {
    int use_perspective = 0;
    float fov_degrees = 60.0F;
    float near_plane = 1.0F;
    float far_plane = 5000.0F;
};

glm::mat4 build_projection_matrix(
    const ViewportFit& fit,
    const ProjectionControls& controls);
