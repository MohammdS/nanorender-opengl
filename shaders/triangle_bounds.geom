#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;

flat out vec3 box_color;

vec3 stable_face_color(int face_index)
{
    float seed = float(face_index + 1);
    vec3 noise = fract(
        sin(seed + vec3(0.0, 17.0, 43.0)) * 43758.5453);
    return vec3(0.25) + noise * 0.70;
}

void emit_corner(vec2 corner, float depth)
{
    gl_Position = vec4(corner, depth, 1.0);
    EmitVertex();
}

void main()
{
    if (gl_in[0].gl_Position.w <= 0.0
        || gl_in[1].gl_Position.w <= 0.0
        || gl_in[2].gl_Position.w <= 0.0) {
        return;
    }

    vec3 a = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 b = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
    vec3 c = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;
    vec2 minimum = clamp(min(a.xy, min(b.xy, c.xy)), -1.0, 1.0);
    vec2 maximum = clamp(max(a.xy, max(b.xy, c.xy)), -1.0, 1.0);
    float depth = clamp((a.z + b.z + c.z) / 3.0, -1.0, 1.0);

    box_color = stable_face_color(gl_PrimitiveIDIn);
    emit_corner(vec2(minimum.x, minimum.y), depth);
    emit_corner(vec2(maximum.x, minimum.y), depth);
    emit_corner(vec2(minimum.x, maximum.y), depth);
    emit_corner(vec2(maximum.x, maximum.y), depth);
    EndPrimitive();
}
