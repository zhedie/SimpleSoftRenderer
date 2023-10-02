#include "renderer.h"

Renderer::Renderer() {
    tracerSettings = &tracer.settings;
}

void Renderer::resize(uint32_t width, uint32_t height) {
    tracer.resize(width, height);
    rasterizer.resize(width, height);
}

void Renderer::render(const Scene &scene, const Camera &camera) {
    if (rendererSettings.renderingMode == RenderingMode::Rasterization) {
        rasterizer.render(scene, camera);
        image = rasterizer.getImage();
    }
    else if (rendererSettings.renderingMode == RenderingMode::RayTracing) {
        tracer.render(scene, camera);
        image = tracer.getImage();
    }
}

void Renderer::resetTracerFrame() {
    tracer.resetFrame();
}
