#include "mesh.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::uint32_t parse_vertex_index(
    const std::string& token,
    std::size_t vertex_count,
    std::size_t line_number)
{
    const std::size_t slash = token.find('/');
    const std::string index_text = token.substr(0, slash);
    if (index_text.empty()) {
        throw std::runtime_error(
            "OBJ face has an empty vertex index on line "
            + std::to_string(line_number));
    }

    std::size_t parsed_characters = 0;
    long long obj_index = 0;
    try {
        obj_index = std::stoll(index_text, &parsed_characters);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "OBJ face has an invalid vertex index on line "
            + std::to_string(line_number));
    }
    if (parsed_characters != index_text.size() || obj_index == 0) {
        throw std::runtime_error(
            "OBJ face has an invalid vertex index on line "
            + std::to_string(line_number));
    }

    const long long resolved_index = obj_index > 0
        ? obj_index - 1
        : static_cast<long long>(vertex_count) + obj_index;
    if (resolved_index < 0
        || resolved_index >= static_cast<long long>(vertex_count)
        || resolved_index
            > static_cast<long long>(
                std::numeric_limits<std::uint32_t>::max())) {
        throw std::runtime_error(
            "OBJ face index is out of range on line "
            + std::to_string(line_number));
    }

    return static_cast<std::uint32_t>(resolved_index);
}

} // namespace

Mesh load_obj_mesh(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open OBJ file: " + path.string());
    }

    Mesh mesh;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(file, line)) {
        ++line_number;
        std::istringstream stream(line);
        std::string record_type;
        stream >> record_type;
        if (record_type.empty() || record_type.starts_with('#')) {
            continue;
        }

        if (record_type == "v") {
            glm::vec3 vertex {0.0F};
            if (!(stream >> vertex.x >> vertex.y >> vertex.z)) {
                throw std::runtime_error(
                    "OBJ vertex is incomplete on line "
                    + std::to_string(line_number));
            }
            mesh.vertices.push_back(vertex);
            continue;
        }

        if (record_type == "f") {
            std::vector<std::uint32_t> polygon;
            std::string token;
            while (stream >> token) {
                if (token.starts_with('#')) {
                    break;
                }
                polygon.push_back(parse_vertex_index(
                    token,
                    mesh.vertices.size(),
                    line_number));
            }
            if (polygon.size() < 3) {
                throw std::runtime_error(
                    "OBJ face has fewer than three vertices on line "
                    + std::to_string(line_number));
            }

            for (std::size_t index = 1; index + 1 < polygon.size(); ++index) {
                mesh.faces.push_back(
                    {{polygon[0], polygon[index], polygon[index + 1]}});
            }
        }
    }

    if (mesh.vertices.empty() || mesh.faces.empty()) {
        throw std::runtime_error(
            "OBJ file must contain at least one vertex and one face: "
            + path.string());
    }
    return mesh;
}

MeshNormals calculate_mesh_normals(const Mesh& mesh)
{
    MeshNormals normals;
    normals.face_normals.reserve(mesh.faces.size());
    normals.vertex_normals.assign(mesh.vertices.size(), glm::vec3(0.0F));

    const auto normalize_or_zero = [](const glm::vec3& value) {
        const float length = glm::length(value);
        return length > std::numeric_limits<float>::epsilon()
            ? value / length
            : glm::vec3(0.0F);
    };

    for (const TriangleFace& face : mesh.faces) {
        for (const std::uint32_t index : face.indices) {
            if (index >= mesh.vertices.size()) {
                throw std::runtime_error(
                    "Cannot calculate normals with an invalid face index.");
            }
        }

        const glm::vec3& a = mesh.vertices[face.indices[0]];
        const glm::vec3& b = mesh.vertices[face.indices[1]];
        const glm::vec3& c = mesh.vertices[face.indices[2]];
        const glm::vec3 face_normal =
            normalize_or_zero(glm::cross(b - a, c - a));
        normals.face_normals.push_back(face_normal);
        for (const std::uint32_t index : face.indices) {
            normals.vertex_normals[index] += face_normal;
        }
    }

    for (glm::vec3& vertex_normal : normals.vertex_normals) {
        vertex_normal = normalize_or_zero(vertex_normal);
    }
    return normals;
}

MeshBounds calculate_mesh_bounds(const Mesh& mesh)
{
    if (mesh.vertices.empty()) {
        throw std::runtime_error("Cannot calculate bounds for an empty mesh.");
    }

    MeshBounds bounds {mesh.vertices.front(), mesh.vertices.front()};
    for (const glm::vec3& vertex : mesh.vertices) {
        bounds.min = glm::min(bounds.min, vertex);
        bounds.max = glm::max(bounds.max, vertex);
    }
    return bounds;
}

ViewportFit calculate_viewport_fit(
    const Mesh& mesh,
    int viewport_width,
    int viewport_height,
    float viewport_coverage)
{
    if (viewport_width <= 0 || viewport_height <= 0) {
        throw std::runtime_error("Viewport dimensions must be positive.");
    }
    if (!(viewport_coverage > 0.0F && viewport_coverage <= 1.0F)) {
        throw std::runtime_error("Viewport coverage must be in (0, 1].");
    }

    ViewportFit fit;
    fit.bounds = calculate_mesh_bounds(mesh);
    fit.center = (fit.bounds.min + fit.bounds.max) * 0.5F;
    fit.size = fit.bounds.max - fit.bounds.min;
    const float largest_dimension =
        std::max({fit.size.x, fit.size.y, fit.size.z});
    if (largest_dimension <= std::numeric_limits<float>::epsilon()) {
        throw std::runtime_error("Cannot fit a zero-size mesh to the viewport.");
    }

    const float smaller_viewport_dimension = static_cast<float>(
        std::min(viewport_width, viewport_height));
    fit.uniform_scale =
        viewport_coverage * smaller_viewport_dimension / largest_dimension;
    fit.translation = {
        static_cast<float>(viewport_width) * 0.5F
            - fit.center.x * fit.uniform_scale,
        static_cast<float>(viewport_height) * 0.5F
            - fit.center.y * fit.uniform_scale,
        -fit.center.z * fit.uniform_scale,
    };
    if (std::abs(fit.translation.z)
        <= std::numeric_limits<float>::epsilon()) {
        fit.translation.z = 0.0F;
    }
    fit.viewport_width = viewport_width;
    fit.viewport_height = viewport_height;
    return fit;
}

glm::vec3 apply_viewport_fit(
    const glm::vec3& vertex,
    const ViewportFit& fit)
{
    return vertex * fit.uniform_scale + fit.translation;
}
