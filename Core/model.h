#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include "mesh.h"

Texture textureFromFile(const char *path, const std::string &directory);

class Model {
    void loadModel(const std::string &path);

    void processNode(aiNode *node, const aiScene *scene);
    
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName);

public:
    std::vector<Texture> texturesLoaded;
    std::vector<Mesh> meshes;
    std::string directory;
    glm::vec3 scale{1.0f};
    glm::vec3 translate{0.0f};

    Model(const std::string &path);
};