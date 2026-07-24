#version 410 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_viewport_fit;
uniform mat4 u_local_transform;
uniform mat4 u_world_transform;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 view_position;

void main()
{
    vec4 position = u_view * u_world_transform
        * u_local_transform * u_viewport_fit * vec4(a_position, 1.0);
    view_position = position.xyz;
    gl_Position = u_projection * position;
}
