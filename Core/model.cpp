#include "model.h"
#include <iostream>

static Texture textureFromFile(const char *filename, const std::string &directory) {
    std::string path = std::string(filename);
    path = directory + '\\' + path;
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

    Texture texture;
    texture.data.resize(width * height);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    else {
        for (int i = 0; i < width * height; ++i) {
            if (nrComponents == 1) {
                texture.data[i] = glm::vec4((float)data[i] / 255.0f, 0.0f, 0.0f, 1.0f);
            }
            else if (nrComponents == 3) {
                texture.data[i] = glm::vec4((float)data[3 * i] / 255.0f, 
                                            (float)data[3 * i + 1] / 255.0f, 
                                            (float)data[3 * i + 2] / 255.0f, 
                                            1.0f);
            }
            else if (nrComponents == 4) {
                texture.data[i] = glm::vec4((float)data[4 * i] / 255.0f, 
                                            (float)data[4 * i + 1] / 255.0f, 
                                            (float)data[4 * i + 2] / 255.0f, 
                                            (float)data[4 * i + 3] / 255.0f);
                
            }
        }
        texture.width = width;
        texture.height = height;
    }
    stbi_image_free(data);
    return texture;
}

void Model::loadModel(const std::string &path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: assimp load model failed.(Failed to init importer)" << std::endl;
        return;
    }
    size_t n = path.find_last_of('/');
    if (n >= path.size()) {
        n = path.find_last_of('\\');
    }
    directory = path.substr(0, n);
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material mat;
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }
        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent.x = mesh->mTangents[i].x;   
            vertex.tangent.y = mesh->mTangents[i].y;   
            vertex.tangent.z = mesh->mTangents[i].z;
            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;
        }
        if (mesh->mTextureCoords[0]) {
            vertex.texcoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texcoords.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            vertex.texcoords = glm::vec2(0, 0);
        }
        

        vertices.emplace_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        // I'll assume that all faces are triangles here
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.emplace_back(face.mIndices[j]);
        }
    }
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    mat.diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    mat.specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    // mat.bumpMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_bump");
    // mat.normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    
    // .mtl file map_Bump reference to normal map
    mat.normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    mat.displacementMaps = loadMaterialTextures(material, aiTextureType_DISPLACEMENT, "texture_displacement");

    // std::cout << "diffuse maps: " << mat.diffuseMaps.size() << std::endl;
    // std::cout << "specular maps: " << mat.specularMaps.size() << std::endl;
    // std::cout << "bump maps: " << mat.bumpMaps.size() << std::endl;
    // std::cout << "normal maps: " << mat.normalMaps.size() << std::endl;
    // std::cout << "displacement maps: " << mat.displacementMaps.size() << std::endl;

    aiColor3D color;
    float Ns = 0.0;
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color)) {
        mat.ka = glm::vec3(color.r, color.g, color.b);
    }
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
        mat.kd = glm::vec3(color.r, color.g, color.b);
    }
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
        mat.ks = glm::vec3(color.r, color.g, color.b);
    }
    if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, Ns)) {
        mat.ns = Ns;
    }
    // std::cout << "ka = " << mat.ka.x << ' ' << mat.ka.y << ' ' << mat.ka.z << std::endl;
    // std::cout << "kd = " << mat.kd.x << ' ' << mat.kd.y << ' ' << mat.kd.z << std::endl;
    // std::cout << "ks = " << mat.ks.x << ' ' << mat.ks.y << ' ' << mat.ks.z << std::endl;
    
    return Mesh{vertices, indices, mat};
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (const auto &texLoaded : texturesLoaded) {
            if (std::strcmp(texLoaded.file.data(), str.C_Str()) == 0) {
                textures.emplace_back(texLoaded);
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture texture;
            texture = textureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.file = str.C_Str();
            textures.emplace_back(texture);
            texturesLoaded.emplace_back(texture);
        }
        
    }
    return textures;
}

Model::Model(const std::string &path) {
    loadModel(path);
}