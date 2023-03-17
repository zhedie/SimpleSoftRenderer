#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include "Shader.h"

struct V2F_Triangle {
    std::array<V2F, 3> vertices;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Eigen::Vector3f ka;
    Eigen::Vector3f kd;
    Eigen::Vector3f ks;
    float Ns;

    Mesh();
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Eigen::Vector3f ka, Eigen::Vector3f kd, Eigen::Vector3f ks, float Ns);
};