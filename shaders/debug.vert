#version 410 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;

uniform mat4 u_viewport_fit;
uniform mat4 u_local_transform;
uniform mat4 u_world_transform;
uniform mat4 u_projection;

out vec3 line_color;

void main()
{
    line_color = a_color;
    gl_Position = u_projection * u_world_transform * u_local_transform
        * u_viewport_fit * vec4(a_position, 1.0);
}
