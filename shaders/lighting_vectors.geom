#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 24) out;

in vec3 view_position[];
flat out vec3 line_color;

uniform mat4 u_projection;
uniform vec3 u_light_position_view;
uniform vec2 u_viewport_size;
uniform float u_vector_length;
uniform float u_line_half_width;
uniform int u_face_limit;

vec3 normalize_or_zero(vec3 value)
{
    float magnitude = length(value);
    return magnitude > 0.000001 ? value / magnitude : vec3(0.0);
}

void emit_thick_line(vec3 start, vec3 end, vec3 color)
{
    vec4 start_clip = u_projection * vec4(start, 1.0);
    vec4 end_clip = u_projection * vec4(end, 1.0);
    if (start_clip.w <= 0.0 || end_clip.w <= 0.0) {
        return;
    }

    vec2 start_ndc = start_clip.xy / start_clip.w;
    vec2 end_ndc = end_clip.xy / end_clip.w;
    vec2 pixel_direction =
        (end_ndc - start_ndc) * u_viewport_size * 0.5;
    float pixel_length = length(pixel_direction);
    if (pixel_length <= 0.0001) {
        return;
    }

    vec2 perpendicular =
        vec2(-pixel_direction.y, pixel_direction.x) / pixel_length;
    vec2 offset_ndc =
        perpendicular * u_line_half_width * 2.0 / u_viewport_size;
    vec4 start_offset =
        vec4(offset_ndc * start_clip.w, 0.0, 0.0);
    vec4 end_offset =
        vec4(offset_ndc * end_clip.w, 0.0, 0.0);

    line_color = color;
    gl_Position = start_clip + start_offset;
    EmitVertex();
    gl_Position = start_clip - start_offset;
    EmitVertex();
    gl_Position = end_clip + end_offset;
    EmitVertex();
    gl_Position = end_clip - end_offset;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    if (gl_PrimitiveIDIn >= u_face_limit) {
        return;
    }

    vec3 face_center =
        (view_position[0] + view_position[1] + view_position[2]) / 3.0;
    vec3 face_normal = normalize_or_zero(
        cross(
            view_position[1] - view_position[0],
            view_position[2] - view_position[0]));
    vec3 toward_light =
        normalize_or_zero(u_light_position_view - face_center);
    vec3 incident = -toward_light;
    vec3 reflection = normalize_or_zero(
        incident - 2.0 * dot(face_normal, incident) * face_normal);

    emit_thick_line(
        face_center + toward_light * u_vector_length,
        face_center,
        vec3(1.0, 0.46, 0.06));
    emit_thick_line(
        face_center,
        face_center + reflection * u_vector_length,
        vec3(1.0, 0.16, 0.82));
}
