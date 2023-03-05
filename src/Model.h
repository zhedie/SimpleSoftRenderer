#pragma once
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>
#include <iostream>

Texture texture_from_file(const char *path, const std::string &directory);

class Model {
    void load_model(const std::string &path);

    void process_node(aiNode *node, const aiScene *scene);

    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name);

public:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;

    Model(const std::string &path);

    void render(FrameBuf *buf, VertexShader &vs, FragmentShader &fs);
};