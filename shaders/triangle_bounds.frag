#version 410 core

flat in vec3 box_color;

layout(location = 0) out vec4 fragment_color;

void main()
{
    fragment_color = vec4(box_color, 1.0);
}
