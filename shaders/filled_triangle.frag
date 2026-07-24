#version 410 core

in vec3 barycentric_weights;
flat in vec3 face_color;
in float view_depth;

uniform int u_show_barycentric;
uniform int u_show_depth;
uniform float u_near_plane;
uniform float u_far_plane;

layout(location = 0) out vec4 fragment_color;

void main()
{
    if (u_show_depth != 0) {
        float depth_range = max(u_far_plane - u_near_plane, 0.0001);
        float normalized_depth =
            clamp((view_depth - u_near_plane) / depth_range, 0.0, 1.0);
        float grayscale =
            1.0 - smoothstep(0.10, 0.30, normalized_depth);
        fragment_color = vec4(vec3(grayscale), 1.0);
        return;
    }

    vec3 color = face_color;
    if (u_show_barycentric != 0) {
        vec3 weights = clamp(barycentric_weights, 0.0, 1.0);
        color = mix(face_color, weights, 0.75);
    }
    fragment_color = vec4(color, 1.0);
}
