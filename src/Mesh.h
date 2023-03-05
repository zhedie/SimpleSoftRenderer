#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include "Shader.h"
#include "FrameBuf.h"

struct V2F_Triangle {
    std::array<V2F, 3> vertices;
};

class Mesh {


    void rasterize_triangles(V2F_Triangle &t, std::array<Eigen::Vector3f, 3> &viewspace_pos, FrameBuf *buf, FragmentShader &fs);
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Eigen::Vector3f ka;
    Eigen::Vector3f kd;
    Eigen::Vector3f ks;

    Mesh();
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Eigen::Vector3f ka, Eigen::Vector3f kd, Eigen::Vector3f ks);
    

    void render(FrameBuf *buf, VertexShader &vs, FragmentShader &fs);
};