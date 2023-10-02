#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <iostream>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    std::vector<glm::vec4> data;
    int width, height;
    std::string type;
    std::string file;

    glm::vec4 getValue(float u, float v) const {
        while (u < 0.0f) {
            u += 1.0f;
        }
        while (u > 1.0f) {
            u -= 1.0f;
        }
        while (v < 0.0f) {
            v += 1.0f;
        }
        while (v > 1.0f) {
            v -= 1.0f;
        }
        int index = std::clamp(static_cast<int>((width - 1) * u), 0, width - 1) + std::clamp(static_cast<int>(v * (height - 1)) * width, 0, (height - 1) * width);
        return data[index];
    }
};

struct Material {
    glm::vec3 albedo{1.0f};

    glm::vec3 ka{0.1f};
    glm::vec3 kd{0.5f};
    glm::vec3 ks{0.5f};
    float ns = 12;

    std::vector<Texture> diffuseMaps;
    std::vector<Texture> specularMaps;
    std::vector<Texture> bumpMaps;
    std::vector<Texture> normalMaps;
    std::vector<Texture> displacementMaps;

    // only for tracer
    glm::vec3 emissionColor{1.0f};
    float emissionPower{0.0f};

    glm::vec3 getEmission() const {
        // return glm::clamp(emissionColor * emissionPower, glm::vec3{0.0f}, glm::vec3{1.0f});
        return emissionColor * emissionPower;
    }
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material mat;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material mat) {
        this->vertices = vertices;
        this->indices = indices;
        this->mat = mat;
    }
};

