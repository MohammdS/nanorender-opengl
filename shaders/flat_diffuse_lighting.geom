#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 view_position[];
flat out vec3 face_color;

uniform vec3 u_light_position_view;
uniform vec3 u_light_ambient;
uniform vec3 u_light_diffuse;
uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;

vec3 normalize_or_zero(vec3 value)
{
    float magnitude = length(value);
    return magnitude > 0.000001 ? value / magnitude : vec3(0.0);
}

void main()
{
    vec3 face_center =
        (view_position[0] + view_position[1] + view_position[2]) / 3.0;
    vec3 face_normal = normalize_or_zero(
        cross(
            view_position[1] - view_position[0],
            view_position[2] - view_position[0]));
    vec3 light_direction =
        normalize_or_zero(u_light_position_view - face_center);
    float lambert = max(dot(face_normal, light_direction), 0.0);
    vec3 ambient = u_light_ambient * u_material_ambient;
    vec3 diffuse =
        u_light_diffuse * u_material_diffuse * lambert;
    face_color = clamp(ambient + diffuse, 0.0, 1.0);

    for (int corner = 0; corner < 3; ++corner) {
        gl_Position = gl_in[corner].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
