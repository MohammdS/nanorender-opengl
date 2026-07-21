#version 410 core

in vec2 fragment_texture_coordinate;
in vec4 fragment_vertex_color;

layout(location = 0) out vec4 fragment_color;

uniform sampler2D u_atlas;

void main()
{
    float coverage = texture(u_atlas, fragment_texture_coordinate).r;
    fragment_color = vec4(
        fragment_vertex_color.rgb,
        fragment_vertex_color.a * coverage);
}
