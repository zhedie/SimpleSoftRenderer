#pragma once
#include <glm/glm.hpp>

#include "scene.h"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

struct A2V {
    glm::vec4 position;
    glm::vec4 albedo;
    glm::vec3 normal;
    glm::vec2 texcoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct V2F {
    glm::vec4 position;
    glm::vec3 worldPosition;
    glm::vec4 albedo;
    glm::vec3 normal;
    glm::vec2 texcoords;
    glm::vec3 viewDir;
};

class BasicVertexShader {
    glm::mat4 model{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 projection{1.0f};
public:
    void setModel(const glm::mat4 &_model) {
        model = _model;
    }

    void setView(const glm::mat4 &_view) {
        view = _view;
    }

    void setProjection(const glm::mat4 &_projection) {
        projection = _projection;
    }

    virtual V2F vert(A2V a2v) {
        V2F v2f;
        v2f.position = projection * view * model * a2v.position;
        v2f.worldPosition = model * a2v.position;
        v2f.albedo = a2v.albedo;
        v2f.normal = a2v.normal;
        v2f.texcoords = a2v.texcoords;
        return v2f;
    }
};

class BasicFragmentShader {
public:
    virtual glm::vec4 frag(V2F v2f, const Material &mat, const std::vector<DirectionLight> &lights) {
        glm::vec3 ambient{0.2f};
        glm::vec3 diffuse{0.0f};
        glm::vec3 specular{0.0f};

        glm::vec3 kd = mat.kd;
        glm::vec3 ks = mat.ks;
        glm::vec3 normal = v2f.normal;
        glm::vec3 worldPosition = v2f.worldPosition;
        if (!mat.diffuseMaps.empty()) {
            kd = mat.diffuseMaps[0].getValue(v2f.texcoords.x, v2f.texcoords.y);
        }
        if (!mat.specularMaps.empty()) {
            ks = mat.specularMaps[0].getValue(v2f.texcoords.x, v2f.texcoords.y);
        }

        const glm::vec3 &n = v2f.normal;
        float x = n.x, y = n.y, z = n.z;
        // Construct the tbn matrix, but it is usually calculated through the vertex position and uv coordinates in vs or through APIs such as dFdx in glsl
        glm::vec3 t(x * y / std::sqrt(x * x + z * z), -std::sqrt(x * x + z * z), y * z / std::sqrt(x * x + z * z));
        glm::vec3 b = glm::cross(n, t);
        glm::mat3 tbn(t, b, n);

        if (!mat.normalMaps.empty()) {
            normal = mat.normalMaps[0].getValue(v2f.texcoords.x, v2f.texcoords.y) - 0.5f;
            normal = glm::normalize(tbn * normal);
        }
        float kh = 0.2f, kn = 0.1f;
        if (!mat.displacementMaps.empty() || !mat.bumpMaps.empty()) {
            // just use the first texture
            // displacement map first
            auto &tex = !mat.displacementMaps.empty() ? mat.displacementMaps[0] : mat.bumpMaps[0];
            auto f = [&](const float u, const float v) {
                return glm::normalize(glm::vec3(tex.getValue(u, v)));
            };
            const int w = tex.width;
            const int h = tex.height;
            
            float u = v2f.texcoords.x;
            float v = v2f.texcoords.y;
            // calculate the normal of the vertex position after offset
            float dU = kh * kn * glm::length(f(u + 1.0f / w, v) - f(u, v));
            float dV = kh * kn * glm::length(f(u, v + 1.0f / h) - f(u, v));
            glm::vec3 ln{-dU, -dV, 1.0f};

            if (!mat.displacementMaps.empty()) {
                worldPosition += kn * n * f(u, v);
            }
            normal = glm::normalize(tbn * ln);
        }

        glm::vec3 viewPos = v2f.viewDir + v2f.worldPosition;
        glm::vec3 viewDir = glm::normalize(viewPos - v2f.worldPosition);

        ambient *= mat.ka;
        for (const auto &light : lights) {
            diffuse += kd * light.intensity * std::max(0.0f, glm::dot(glm::normalize(-light.direction), normal));
            glm::vec3 halfVec = glm::normalize(glm::normalize(-light.direction) + viewDir);
            specular += ks * light.intensity * (float)std::pow(std::max(0.0f, glm::dot(halfVec, normal)), mat.ns);
        }
        return glm::vec4(glm::clamp(ambient + diffuse + specular, glm::vec3{0.0f}, glm::vec3{1.0f}), 1.0f);
    }
};