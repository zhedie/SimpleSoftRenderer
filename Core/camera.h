#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera {
    glm::mat4 viewportTransform{1.0f};
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 inverseProjection{1.0f};
    glm::mat4 inverseView{1.0f};

    float verticalFOV = 45.0f;
    float nearClip = 0.1f;
    float farClip = 100.0f;

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 forwardDirection{0.0f, 0.0f, 0.0f};

    std::vector<glm::vec3> rayDirections;

    glm::vec2 lastMousePosition{0.0f, 0.0f};

    uint32_t viewportWidth = 0;
    uint32_t viewportHeight = 0;
private:
    void calculateViewportTransform();
    void calculateProjection();
    void calculateView();
    void calculateRayDirections();
public:
    Camera(float verticalFOV, float nearClip, float farClip);

    bool onUpdate(float ts);
    void onResize(uint32_t width, uint32_t height);

    const glm::mat4& getViewportTransform() const { return viewportTransform; }
    const glm::mat4& getProjection() const { return projection; }
    const glm::mat4& getInverseProjection() const { return inverseProjection; }
    const glm::mat4& getView() const { return view; }
    const glm::mat4& getInverseView() const { return inverseView; }

    const glm::vec3& getPosition() const { return position; }
    const glm::vec3& getDirection() const { return forwardDirection; }

    const std::vector<glm::vec3>& getRayDirections() const { return rayDirections; }

    float getRotationSpeed();

};