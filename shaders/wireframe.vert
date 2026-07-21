#version 410 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_viewport_fit;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_viewport_fit * vec4(a_position, 1.0);
}
