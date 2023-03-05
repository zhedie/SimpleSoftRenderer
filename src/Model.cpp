#include "Model.h"

static Texture texture_from_file(const char *path, const std::string &directory) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;
    int width, height, nr_components;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nr_components, 0);

    Texture texture;
    texture.data.resize(width * height * 4);
    if (data) {
        for (int i = 0; i < width * height; ++i) {
            if (nr_components == 1) {
                texture.data[4 * i] = data[i];
                texture.data[4 * i + 1] = data[i];
                texture.data[4 * i + 2] = data[i];
                texture.data[4 * i + 3] = 0xff;
            }
            else if (nr_components == 3) {
                texture.data[4 * i] = data[3 * i];
                texture.data[4 * i + 1] = data[3 * i + 1];
                texture.data[4 * i + 2] = data[3 * i + 2];
                texture.data[4 * i + 3] = 0xff;
            }
            else if (nr_components == 4) {
                texture.data[4 * i] = data[4 * i];
                texture.data[4 * i + 1] = data[4 * i + 1];
                texture.data[4 * i + 2] = data[4 * i + 2];
                texture.data[4 * i + 3] = data[4 * i + 3];
            }
        }
        texture.width = width;
        texture.height = height;
    }
    else {
        std::cout << "Failed to load texture: " << filename << std::endl;
    }
    stbi_image_free(data);
    return texture;
}


void Model::load_model(const std::string &path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "Error: assimp load model failed" << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(process_mesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        // position
        Eigen::Vector4f position;
        position.x() = mesh->mVertices[i].x;
        position.y() = mesh->mVertices[i].y;
        position.z() = mesh->mVertices[i].z;
        position.w() = 1;
        vertex.position = position;
        // normals
        if (mesh->HasNormals()) {
            Eigen::Vector3f normal;
            normal.x() = mesh->mNormals[i].x;
            normal.y() = mesh->mNormals[i].y;
            normal.z() = mesh->mNormals[i].z;
            vertex.normal = normal;
        }
        // texcoords
        if (mesh->mTextureCoords[0]) {
            Eigen::Vector2f texcoords;
            texcoords.x() = mesh->mTextureCoords[0][i].x;
            texcoords.y() = mesh->mTextureCoords[0][i].y;
            vertex.texcoords = texcoords;
            // tangent
            // bitangent
        }
        else {
            vertex.texcoords = Eigen::Vector2f(0.f, 0.f);
        }

        vertices.emplace_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.emplace_back(face.mIndices[j]);
        }
    }
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
    std::vector<Texture> specular_maps = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
    std::vector<Texture> normal_maps = load_material_textures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
    std::vector<Texture> height_maps = load_material_textures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), height_maps.begin(), height_maps.end());

    aiColor3D color;
    material->Get(AI_MATKEY_COLOR_AMBIENT, color);
    Eigen::Vector3f ka = Eigen::Vector3f(color.r, color.g, color.b);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    Eigen::Vector3f kd = Eigen::Vector3f(color.r, color.g, color.b);
    material->Get(AI_MATKEY_COLOR_SPECULAR, color);
    Eigen::Vector3f ks = Eigen::Vector3f(color.r, color.g, color.b);

    return Mesh(vertices, indices, textures, ka, kd, ks);
}

std::vector<Texture> Model::load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); ++j) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.emplace_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture texture;
            texture = texture_from_file(str.C_Str(), this->directory);
            texture.type = type_name;
            texture.path = str.C_Str();
            textures.emplace_back(texture);
            textures_loaded.emplace_back(texture);
        }
    }
    return textures;
}

Model::Model(const std::string &path) {
    load_model(path);
}

void Model::render(FrameBuf *buf, VertexShader &vs, FragmentShader &fs) {
    for (auto &i : meshes) {
        i.render(buf, vs, fs);
    }
}
