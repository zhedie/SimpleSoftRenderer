#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "application.h"
#include "image.h"
#include "random.h"
#include "timer.h"

#include "renderer.h"
#include "camera.h"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

class RendererLayer : public Layer {
    uint32_t imageWidth = 0, imageHeight = 0;
    Renderer renderer;
    Camera camera;
    Scene scene;
    bool autoRender = false;

    float lastRenderTimeCost = 0;

private:
    void render() {
        Timer timer;

        renderer.resize(imageWidth, imageHeight);
        camera.onResize(imageWidth, imageHeight);
        renderer.render(scene, camera);

        lastRenderTimeCost = timer.elapsedMs();
    }
public:
    RendererLayer() : camera(45.0f, 0.1f, 100.f) {

        DirectionLight light;
        light.direction = glm::vec3{0, 0, -1};
        light.intensity = glm::vec3{1, 1, 1};

        scene.lights.emplace_back(light);
    }

    virtual void onUpdate(float timestep) override {
        if (camera.onUpdate(timestep)) {
            // camera moved
            renderer.resetTracerFrame();
        }
    }

    virtual void onUIRender() override {
        ImGui::Begin("Settings");
        if (ImGui::CollapsingHeader("Renderer Settings")) {
            {            
                ImGui::Text("Last render time cost: %3.fms", lastRenderTimeCost);
                ImGui::Text("camera position: (%f, %f, %f)", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
            }
            {
                static int prevModeIndex = 0;
                static int renderingModeIndex = 0;
                ImGui::RadioButton("Rasterizer", &renderingModeIndex, 0); ImGui::SameLine();
                ImGui::RadioButton("RayTracer", &renderingModeIndex, 1);
                switch (renderingModeIndex) {
                    case 0:
                        renderer.rendererSettings.renderingMode = Renderer::RenderingMode::Rasterization;
                        break;
                    case 1:
                        renderer.rendererSettings.renderingMode = Renderer::RenderingMode::RayTracing;
                        break;
                    default:
                        break;
                }
                if (renderingModeIndex != prevModeIndex) {
                    prevModeIndex = renderingModeIndex;
                    autoRender = false;
                }
            }
            {
                ImGui::Checkbox("auto render", &autoRender);
            }
        }
        if (ImGui::CollapsingHeader("Rasterizer Settings")) {
            
        }
        if (ImGui::CollapsingHeader("RayTracer Settings")) {
            ImGui::Checkbox("accumulate", &renderer.tracerSettings->accumulate);
            ImGui::DragInt("bounce times", &renderer.tracerSettings->bounceTimes, 1, 2, 10);
        }
        if (ImGui::Button("Render")) {
            render();
        }
        ImGui::End();

        ImGui::Begin("Scene");
        if (ImGui::CollapsingHeader("Objects")) {
            {
                int deleteIndex = -1;
                for (size_t i = 0; i < scene.models.size(); ++i) {
                    ImGui::PushID((int)i);
                    ImGui::Text("Model %u", i); ImGui::SameLine();
                    if (ImGui::Button("delete")) {
                        deleteIndex = (int)i;
                    }
                    ImGui::DragFloat3("scale", glm::value_ptr(scene.models[i].scale), 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat3("translate", glm::value_ptr(scene.models[i].translate), 0.1f);
                    ImGui::PopID();
                }
                if (deleteIndex != -1) {
                    scene.models.erase(scene.models.begin() + deleteIndex);
                }
            }
        }
        if (ImGui::CollapsingHeader("Rasterizer Scene")) {
            {
                ImGui::Text("Lights");
                ImGui::DragFloat3("direction", glm::value_ptr(scene.lights[0].direction), 0.5f);
                ImGui::ColorEdit3("intensity", glm::value_ptr(scene.lights[0].intensity));
            }

        }
        if (ImGui::CollapsingHeader("RayTracing Scene")) {
            {
                ImGui::ColorEdit3("sky color", glm::value_ptr(scene.skyColor));
            }
        }
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Viewport");
        {
            imageWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
            imageHeight = (uint32_t)ImGui::GetContentRegionAvail().y;
            auto image = renderer.getImage();
            if (image) {
                ImGui::Image(image->getDescriptorSet(), {(float)image->getWidth(), (float)image->getHeight()}, ImVec2(0, 1), ImVec2(1, 0));
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();

        openModel();

        // When executing here for the first time, the width and height have not been set, so render() cannot be called at first.
        // if (autoRender && renderer.rendererSettings.renderingMode == Renderer::RenderingMode::Rasterization) {
        if (autoRender) {
            render();
        }
    }

    void openModel() {
        if (ImGuiFileDialog::Instance()->Display("ChooseFile")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                // std::cout << "filePathName = " << filePathName << std::endl;
                // std::cout << "filePath = " << filePath << std::endl;
                scene.models.emplace_back(Model(filePathName));
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
};

Application* createApplication() {
    ApplicationSpecification appSpec;
    appSpec.name = "Demo";

    Application *app = new Application(appSpec);
    std::shared_ptr<RendererLayer> layer = std::make_shared<RendererLayer>();
    // app->pushLayer<RendererLayer>();
    app->pushLayer(layer);
    app->setMenubarCallback([&app, &layer]() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                app->stop();
            }
            if (ImGui::MenuItem("Open")) {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFile", "Choose Model File", ".obj", ".");
            }
            ImGui::EndMenu();
        }
    });
    return app;
}