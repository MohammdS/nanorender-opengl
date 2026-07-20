#include "mesh.h"

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
