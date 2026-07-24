#version 410 core

flat in vec3 face_color;

layout(location = 0) out vec4 fragment_color;

void main()
{
    fragment_color = vec4(face_color, 1.0);
}
