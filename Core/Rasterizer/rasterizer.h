#pragma once
#include "image.h"
#include "camera.h"
#include "scene.h"
#include "shader.h"

#include <memory>
#include <vector>
#include <array>
#include <glm/glm.hpp>

class Rasterizer {
private:

public:

private: 
    std::shared_ptr<Image> image;
    uint32_t *imageData = nullptr;

    std::vector<glm::vec4> colorBuffer;
    std::vector<float> depthBuffer;

    const Camera *activeCamera = nullptr;
    const Scene *activeScene = nullptr;

public:

private:
    void rasterize(std::array<V2F, 3> &v2fs, BasicFragmentShader &fs, const Material &mat, const std::vector<DirectionLight> &lights);
public:
    void resize(uint32_t width, uint32_t height);
    void render(const Scene &scene, const Camera &camera);

    std::shared_ptr<Image> getImage() const { return image; }
};