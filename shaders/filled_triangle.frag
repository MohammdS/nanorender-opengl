#version 410 core

in vec3 barycentric_weights;
flat in vec3 face_color;

uniform int u_show_barycentric;

layout(location = 0) out vec4 fragment_color;

void main()
{
    vec3 color = face_color;
    if (u_show_barycentric != 0) {
        vec3 weights = clamp(barycentric_weights, 0.0, 1.0);
        color = mix(face_color, weights, 0.75);
    }
    fragment_color = vec4(color, 1.0);
}
