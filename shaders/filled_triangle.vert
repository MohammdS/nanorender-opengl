#version 410 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_barycentric;
layout(location = 2) in vec3 a_face_color;

uniform mat4 u_viewport_fit;
uniform mat4 u_local_transform;
uniform mat4 u_world_transform;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 barycentric_weights;
flat out vec3 face_color;
out float view_depth;

void main()
{
    barycentric_weights = a_barycentric;
    face_color = a_face_color;
    vec4 view_position = u_view * u_world_transform
        * u_local_transform * u_viewport_fit * vec4(a_position, 1.0);
    view_depth = max(-view_position.z, 0.0);
    gl_Position = u_projection * view_position;
}
