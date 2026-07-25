#version 410 core

in vec3 view_position;
in vec3 view_normal;

uniform vec3 u_light_position_view;
uniform vec3 u_light_ambient;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;
uniform vec3 u_material_ambient;
uniform vec3 u_material_diffuse;
uniform vec3 u_material_specular;
uniform float u_shininess;

layout(location = 0) out vec4 fragment_color;

vec3 normalize_or_zero(vec3 value)
{
    float magnitude = length(value);
    return magnitude > 0.000001 ? value / magnitude : vec3(0.0);
}

void main()
{
    vec3 normal = normalize_or_zero(view_normal);
    vec3 light_direction =
        normalize_or_zero(u_light_position_view - view_position);
    float lambert = max(dot(normal, light_direction), 0.0);

    vec3 incident = -light_direction;
    vec3 reflection = normalize_or_zero(
        incident - 2.0 * dot(normal, incident) * normal);
    vec3 view_direction = normalize_or_zero(-view_position);
    float reflection_alignment =
        max(dot(reflection, view_direction), 0.0);
    float specular_factor = lambert > 0.0
        ? pow(reflection_alignment, max(u_shininess, 1.0))
        : 0.0;

    vec3 ambient = u_light_ambient * u_material_ambient;
    vec3 diffuse =
        u_light_diffuse * u_material_diffuse * lambert;
    vec3 specular =
        u_light_specular * u_material_specular * specular_factor;
    fragment_color =
        vec4(clamp(ambient + diffuse + specular, 0.0, 1.0), 1.0);
}
