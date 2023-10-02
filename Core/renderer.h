#pragma once
#include "image.h"
#include "camera.h"
#include "scene.h"

#include "tracer.h"
#include "rasterizer.h"

#include <memory>

class Renderer { 
public:
    enum class RenderingMode { Rasterization, RayTracing };
    struct Settings {
        RenderingMode renderingMode = RenderingMode::Rasterization;
    };
private:
    Tracer tracer;
    Rasterizer rasterizer;
    std::shared_ptr<Image> image;
public:
    Tracer::Settings *tracerSettings = nullptr;

    Settings rendererSettings;
public:
    Renderer();

    void resize(uint32_t width, uint32_t height);
    void render(const Scene &scene, const Camera &camera);
    void resetTracerFrame();

    std::shared_ptr<Image> getImage() const { return image; }
};