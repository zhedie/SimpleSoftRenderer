#include "tracer.h"
#include "random.h"
#include "utils.hpp"

#include <execution>
#include <array>

void Tracer::resize(uint32_t width, uint32_t height) {
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

    delete[] accumulationData;
    accumulationData = new glm::vec4[width * height];
    resetFrame();

    imageHorizontalIter.resize(width);
    imageVerticalIter.resize(height);
    std::iota(imageHorizontalIter.begin(), imageHorizontalIter.end(), 0);
    std::iota(imageVerticalIter.begin(), imageVerticalIter.end(), 0);
}

void Tracer::render(const Scene &scene, const Camera &camera) {
    uint32_t height = image->getHeight(), width = image->getWidth();

    activeCamera = &camera;
    activeScene = &scene;

    if (frameIndex == 1) {
        memset(accumulationData, 0, width * height * sizeof(glm::vec4));
    }
#define MULTI_THREAD true
#if MULTI_THREAD
    std::for_each(std::execution::par, imageVerticalIter.begin(), imageVerticalIter.end(), 
        [this, width](uint32_t y) {
            std::for_each(std::execution::par, imageHorizontalIter.begin(), imageHorizontalIter.end(), 
                [this, y, width](uint32_t x) {
                    glm::vec4 color = perPixel(x, y);
                    accumulationData[x + y * width] += color;
                    glm::vec4 accumulatedColor = accumulationData[x + y * width] / (float)frameIndex;
                    accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

                    imageData[x + y * width] = Utils::glmVec4ToUint32t(accumulatedColor);
                }
            );
        }    
    );
#else
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            glm::vec4 color = perPixel(x, y);
            accumulationData[x + y * width] += color;
            glm::vec4 accumulatedColor = accumulationData[x + y * width] / (float)frameIndex;
            accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

            imageData[x + y * width] = Utils::glmVec4ToUint32t(accumulatedColor);
        }
    }
#endif
#undef MULTI_THREAD

    image->setData(imageData);

    if (settings.accumulate) {
        ++frameIndex;
    }
    else {
        frameIndex = 1;
    }
}

glm::vec3 Tracer::shade(Tracer::HitPayload &hitPayload) {
    const Material &mat = activeScene->models[hitPayload.modelIndex].meshes[hitPayload.meshIndex].mat;
    glm::vec3 kd = mat.kd;
    if (!mat.diffuseMaps.empty()) {
        kd = mat.diffuseMaps[0].getValue(hitPayload.texcoords.x, hitPayload.texcoords.y);
    }

    glm::vec3 normal = hitPayload.worldNormal;
    const glm::vec3 &n = hitPayload.worldNormal;
    float x = n.x, y = n.y, z = n.z;
    // Construct the tbn matrix, but it is usually calculated through the vertex position and uv coordinates in vs or through APIs such as dFdx in glsl
    glm::vec3 t(x * y / std::sqrt(x * x + z * z), -std::sqrt(x * x + z * z), y * z / std::sqrt(x * x + z * z));
    glm::vec3 b = glm::cross(n, t);
    glm::mat3 tbn(t, b, n);
    if (!mat.normalMaps.empty()) {
        normal = mat.normalMaps[0].getValue(hitPayload.texcoords.x, hitPayload.texcoords.y) - 0.5f;
        normal = glm::normalize(tbn * normal);
    }
    hitPayload.worldNormal = normal;

    return kd;
}

glm::vec4 Tracer::perPixel(uint32_t x, uint32_t y) {
    Ray ray;
    ray.origin = activeCamera->getPosition();
    ray.direction = activeCamera->getRayDirections()[x + y * image->getWidth()];

    glm::vec3 light = glm::vec3(0, 0, 0);
    glm::vec3 contribution = glm::vec3(1.0f);
    for (int i = 0; i < settings.bounceTimes; ++i) {
        HitPayload hitPayload = traceRay(ray);
        if (hitPayload.modelIndex < 0) {
            light += activeScene->skyColor * contribution;
            break;
        }
        
        const auto &mat = activeScene->models[hitPayload.modelIndex].meshes[hitPayload.meshIndex].mat;
        light += mat.getEmission() * contribution;
        contribution *= shade(hitPayload);
        
        ray.origin = hitPayload.worldPosition + hitPayload.worldNormal * 0.0001f;
        ray.direction = glm::normalize(Random::unitVec3() + hitPayload.worldNormal);
    }
    return glm::vec4(light, 1.0f);
}

static std::tuple<bool, float, float, float> rayIntersectionWithTriangle(const Ray &ray, std::array<Vertex, 3> &tri) {
    glm::vec3 ca = tri[2].position - tri[0].position;
    glm::vec3 cb = tri[2].position - tri[1].position;
    glm::vec3 co = tri[2].position - ray.origin;
    glm::mat3 A{ray.direction, ca, cb};
    float detA = glm::determinant(A);
    if (detA == 0.0f) {
        return {false, 0.0f, 0.0f, 0.0f};
    }
    
    float t = 0, alpha = 0, beta = 0;
    glm::mat3 A1{co, ca, cb};
    t = glm::determinant(A1) / detA;
    if (t < 0) {
        return {false, 0.0f, 0.0f, 0.0f};
    }
    glm::mat3 A2{ray.direction, co, cb};
    alpha = glm::determinant(A2) / detA;
    if (alpha < 0 || alpha > 1) {
        return {false, 0.0f, 0.0f, 0.0f};
    }
    glm::mat3 A3{ray.direction, ca, co};
    beta = glm::determinant(A3) / detA;
    if (beta < 0 || beta > 1 - alpha) {
        return {false, 0.0f, 0.0f, 0.0f};
    }
    return {true, t, alpha, beta};
}

Tracer::HitPayload Tracer::traceRay(const Ray &ray) {
    float hitDistance = std::numeric_limits<float>::max();
    int modelIndex = -1, meshIndex = -1;
    float alpha = 0, beta = 0;
    int triVertexIndex[3] = {-1, -1, -1};
    for (size_t i = 0; i < activeScene->models.size(); ++i) {
        const auto &model = activeScene->models[i];
        glm::mat4 modelTransform{1.0f};
        modelTransform = glm::scale(modelTransform, model.scale);
        modelTransform = glm::translate(modelTransform, model.translate);
        for (size_t j = 0; j < model.meshes.size(); ++j) {
            const auto &mesh = model.meshes[j];
            for (size_t k = 0; k + 2 < mesh.indices.size(); k += 3) {
                std::array<Vertex, 3> tri{mesh.vertices[mesh.indices[k]], mesh.vertices[mesh.indices[k + 1]], mesh.vertices[mesh.indices[k + 2]]};
                for (auto &v : tri) {
                    v.position = glm::vec3(modelTransform * glm::vec4(v.position, 1.0f));
                }
                auto [intersectRes, t, a, b] = rayIntersectionWithTriangle(ray, tri);
                if (!intersectRes || t > hitDistance) {
                    // no intersection or be covered
                    continue;
                }
                hitDistance = t;
                modelIndex = (int)i;
                meshIndex = (int)j;
                alpha = a;
                beta = b;
                triVertexIndex[0] = (int)k;
                triVertexIndex[1] = (int)(k + 1);
                triVertexIndex[2] = (int)(k + 2);
            }

        }
    }
    if (modelIndex < 0 || meshIndex < 0) {
        return miss();
    }
    else {
        const auto &mesh = activeScene->models[modelIndex].meshes[meshIndex];
        std::array<Vertex, 3> tri = {mesh.vertices[mesh.indices[triVertexIndex[0]]], mesh.vertices[mesh.indices[triVertexIndex[1]]], mesh.vertices[mesh.indices[triVertexIndex[2]]]};
        return closestHit(ray, hitDistance, modelIndex, meshIndex, tri, alpha, beta);
    }
}

Tracer::HitPayload Tracer::closestHit(const Ray &ray, float hitDistance, int modelIndex, int meshIndex, std::array<Vertex, 3> &tri, float alpha, float beta) {
    HitPayload hitPayload;
    hitPayload.hitDistance = hitDistance;
    hitPayload.worldPosition = ray.origin + ray.direction * hitDistance;
    hitPayload.worldNormal = Utils::lerp(alpha, beta, tri[0].position, tri[1].position, tri[2].position);
    hitPayload.texcoords = Utils::lerp(alpha, beta, tri[0].texcoords, tri[1].texcoords, tri[2].texcoords);
    hitPayload.modelIndex = modelIndex;
    hitPayload.meshIndex = meshIndex;

    return hitPayload;
}

Tracer::HitPayload Tracer::miss() {
    HitPayload hitPayload;
    hitPayload.hitDistance = -1.0f;
    hitPayload.modelIndex = -1;
    hitPayload.meshIndex = -1;
    return hitPayload;
}

void Tracer::resetFrame() {
    frameIndex = 1;
}
