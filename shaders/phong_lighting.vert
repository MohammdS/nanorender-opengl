#version 410 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_viewport_fit;
uniform mat4 u_local_transform;
uniform mat4 u_world_transform;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 view_position;
out vec3 view_normal;

void main()
{
    mat4 model =
        u_world_transform * u_local_transform * u_viewport_fit;
    mat4 model_view = u_view * model;
    vec4 position = model_view * vec4(a_position, 1.0);
    mat3 normal_matrix = transpose(inverse(mat3(model_view)));

    view_position = position.xyz;
    view_normal = normalize(normal_matrix * a_normal);
    gl_Position = u_projection * position;
}
