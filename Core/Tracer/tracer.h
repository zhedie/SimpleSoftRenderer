#pragma once

#include "image.h"
#include "camera.h"
#include "geometry.h"
#include "scene.h"
#include "model.h"

#include <memory>

#include <glm/glm.hpp>

class Tracer {
private:
    struct HitPayload {
        float hitDistance;
        glm::vec3 worldPosition;
        glm::vec3 worldNormal;
        glm::vec2 texcoords;

        int modelIndex;
        int meshIndex;
    };
public:
    struct Settings {
        bool accumulate = false;
        int bounceTimes = 2;
    };

private:
    std::shared_ptr<Image> image;
    uint32_t *imageData = nullptr;
    glm::vec4 *accumulationData = nullptr;
    int frameIndex = -1;

    const Camera *activeCamera = nullptr;
    const Scene *activeScene = nullptr;

    std::vector<uint32_t> imageHorizontalIter;
    std::vector<uint32_t> imageVerticalIter;
public:
    Settings settings;

private:
    glm::vec4 perPixel(uint32_t x, uint32_t y);

    glm::vec3 shade(HitPayload &hitPayload);

    HitPayload traceRay(const Ray &ray);
    HitPayload closestHit(const Ray &ray, float hitDistance, int modelIndex, int meshIndex, std::array<Vertex, 3> &tri, float alpha, float beta);
    HitPayload miss();
public:
    Tracer() = default;

    void resize(uint32_t width, uint32_t height);
    void render(const Scene &scene, const Camera &camera);
    void resetFrame();


    std::shared_ptr<Image> getImage() const { return image; }
};