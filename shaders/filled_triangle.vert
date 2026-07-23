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

void main()
{
    barycentric_weights = a_barycentric;
    face_color = a_face_color;
    gl_Position = u_projection * u_view * u_world_transform
        * u_local_transform * u_viewport_fit * vec4(a_position, 1.0);
}
