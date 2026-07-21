#version 410 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texture_coordinate;
layout(location = 2) in vec4 color;

out vec2 fragment_texture_coordinate;
out vec4 fragment_vertex_color;

uniform vec2 u_viewport_size;

void main()
{
    vec2 ndc = vec2(
        position.x * 2.0 / u_viewport_size.x - 1.0,
        1.0 - position.y * 2.0 / u_viewport_size.y);
    gl_Position = vec4(ndc, 0.0, 1.0);
    fragment_texture_coordinate = texture_coordinate;
    fragment_vertex_color = color;
}
