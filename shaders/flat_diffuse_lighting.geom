#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 view_position[];
flat out vec3 face_color;

uniform vec3 u_light_position_view;
uniform vec3 u_light_ambient;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;
uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;
uniform vec3 u_material_specular;
uniform float u_shininess;
uniform int u_enable_specular;

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
    vec3 incident = -light_direction;
    vec3 reflection = normalize_or_zero(
        incident - 2.0 * dot(face_normal, incident) * face_normal);
    vec3 view_direction = normalize_or_zero(-face_center);
    float reflection_alignment =
        max(dot(reflection, view_direction), 0.0);
    float specular_factor =
        u_enable_specular != 0 && lambert > 0.0
        ? pow(reflection_alignment, max(u_shininess, 1.0))
        : 0.0;
    vec3 specular =
        u_light_specular * u_material_specular * specular_factor;
    face_color = clamp(ambient + diffuse + specular, 0.0, 1.0);

    for (int corner = 0; corner < 3; ++corner) {
        gl_Position = gl_in[corner].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
