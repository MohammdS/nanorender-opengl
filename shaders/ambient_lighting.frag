#version 410 core

uniform vec3 u_light_ambient;
uniform vec3 u_material_ambient;

layout(location = 0) out vec4 fragment_color;

void main()
{
    vec3 ambient =
        clamp(u_light_ambient * u_material_ambient, 0.0, 1.0);
    fragment_color = vec4(ambient, 1.0);
}
