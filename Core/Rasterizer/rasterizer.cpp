#include "rasterizer.h"
#include "shader.h"
#include "utils.hpp"

#include <iostream>

void Rasterizer::resize(uint32_t width, uint32_t height) {
    if (!image) {
        image = std::make_shared<Image>(width, height, ImageFormat::RGBA);
    }
    else if (width != image->getWidth() || height != image->getHeight()) {
        image->resize(width, height);
    }
    else {
        return;
    }

    delete[] imageData;
    imageData = new uint32_t[width * height];

    colorBuffer.resize(width * height);
    depthBuffer.resize(width * height);
}

void Rasterizer::rasterize(std::array<V2F, 3> &v2fs, BasicFragmentShader &fs, const Material &mat, const std::vector<DirectionLight> &lights) {
    const int width = image->getWidth();
    const int height = image->getHeight();

    int left = (int)(std::min(std::min(v2fs[0].position.x, v2fs[1].position.x), v2fs[2].position.x));
    int right = (int)(std::max(std::max(v2fs[0].position.x, v2fs[1].position.x), v2fs[2].position.x));
    int bottom = (int)(std::min(std::min(v2fs[0].position.y, v2fs[1].position.y), v2fs[2].position.y));
    int top = (int)(std::max(std::max(v2fs[0].position.y, v2fs[1].position.y), v2fs[2].position.y));

    left = std::max(0, left);
    right = std::min(width - 1, right);
    bottom = std::max(0, bottom);
    top = std::min(height - 1, top);    

    for (int y = bottom; y <= top; ++y) {
        for (int x = left; x <= right; ++x) {
            float t1 = (v2fs[0].position.x - x) * (v2fs[1].position.y - y) - (v2fs[0].position.y - y) * (v2fs[1].position.x - x);
            float t2 = (v2fs[1].position.x - x) * (v2fs[2].position.y - y) - (v2fs[1].position.y - y) * (v2fs[2].position.x - x);
            float t3 = (v2fs[2].position.x - x) * (v2fs[0].position.y - y) - (v2fs[2].position.y - y) * (v2fs[0].position.x - x);

            // back culling
            if (t1 >= 0 && t2 >= 0 && t3 >= 0) {
                int index = y * width + x;
                float alpha = t2 / (t1 + t2 + t3) / v2fs[0].position[3];
                float beta = t3 / (t1 + t2 + t3) / v2fs[1].position[3];
                float gamma = t1 / (t1 + t2 + t3) / v2fs[2].position[3];
                float zCorrection = 1.0f / (alpha + beta + gamma);
                // early-z
                if (zCorrection > depthBuffer[index]) {
                    continue;
                }

                V2F v2f;
                v2f.albedo = zCorrection * Utils::lerp(alpha, beta, gamma, v2fs[0].albedo, v2fs[1].albedo, v2fs[2].albedo);
                v2f.normal = zCorrection * Utils::lerp(alpha, beta, gamma, v2fs[0].normal, v2fs[1].normal, v2fs[2].normal);
                v2f.position = zCorrection * Utils::lerp(alpha, beta, gamma, v2fs[0].position, v2fs[1].position, v2fs[2].position);
                v2f.worldPosition = zCorrection * Utils::lerp(alpha, beta, gamma, v2fs[0].worldPosition, v2fs[1].worldPosition, v2fs[2].worldPosition);
                v2f.texcoords = zCorrection * Utils::lerp(alpha, beta, gamma, v2fs[0].texcoords, v2fs[1].texcoords, v2fs[2].texcoords);
                v2f.viewDir = activeCamera->getPosition() - v2f.worldPosition;
                
                glm::vec4 color = fs.frag(v2f, mat, lights);
                // todo: alpha/stencil
                colorBuffer[index] = color;
                depthBuffer[index] = std::min(depthBuffer[index], zCorrection);
            }
        }
    }
}

void Rasterizer::render(const Scene &scene, const Camera &camera) {
    activeCamera = &camera;
    activeScene = &scene;
    // clear buffer
    std::fill(colorBuffer.begin(), colorBuffer.end(), glm::vec4(0, 0, 0, 1));
    std::fill(depthBuffer.begin(), depthBuffer.end(), std::numeric_limits<float>::max());
    BasicVertexShader vs;
    BasicFragmentShader fs;
    // set view and projection transformation matrix in vs
    vs.setView(camera.getView());
    vs.setProjection(camera.getProjection());
    for (const auto &model : scene.models) {
        glm::mat4 modelTransform{1.0f};
        modelTransform = glm::scale(modelTransform, model.scale);
        modelTransform = glm::translate(modelTransform, model.translate);
        vs.setModel(modelTransform);

        for (const auto &mesh : model.meshes) {
            std::vector<V2F> v2fs;
            for (const auto &vertex : mesh.vertices) {
                A2V a2v;
                a2v.position = glm::vec4(vertex.position, 1.0f);
                a2v.albedo = glm::vec4(1.0f);
                a2v.normal = vertex.normal;
                a2v.texcoords = vertex.texcoords;
                a2v.tangent = vertex.tangent;
                a2v.bitangent = vertex.bitangent;

                v2fs.emplace_back(vs.vert(a2v));
                auto &v2f = v2fs.back();
                float w = v2f.position.w;
                v2f.position /= w;
                v2f.position = camera.getViewportTransform() * v2f.position;
                v2f.position.w = w;

                v2f.viewDir = v2f.worldPosition - camera.getPosition();
            }
            int numIndices = (int)mesh.indices.size();
            for (int i = 0; i + 2 < numIndices; i += 3) {
                std::array<V2F, 3> primitive{v2fs[mesh.indices[i]], v2fs[mesh.indices[i + 1]], v2fs[mesh.indices[i + 2]]};
                const auto &mat = mesh.mat;
                rasterize(primitive, fs, mat, scene.lights);
            }
        }
    }
    for (uint32_t i = 0; i < image->getWidth() * image->getHeight(); ++i) {
        imageData[i] = Utils::glmVec4ToUint32t(colorBuffer[i]);
    }
    image->setData(imageData);
}