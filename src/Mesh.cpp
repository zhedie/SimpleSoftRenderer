#include "Mesh.h"

Mesh::Mesh() {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) 
    : vertices(vertices), indices(indices), textures(textures) {
    ka = Eigen::Vector3f(0.f, 0.f, 0.f);
    kd = Eigen::Vector3f(0.f, 0.f, 0.f);
    ks = Eigen::Vector3f(0.f, 0.f, 0.f);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Eigen::Vector3f ka, Eigen::Vector3f kd, Eigen::Vector3f ks, float Ns) 
    : vertices(vertices), indices(indices), textures(textures), ka(ka), kd(kd), ks(ks), Ns(Ns) {

}