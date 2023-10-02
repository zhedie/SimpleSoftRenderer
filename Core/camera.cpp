#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "input.h"

Camera::Camera(float verticalFOV, float nearClip, float farClip) 
: verticalFOV(verticalFOV), nearClip(nearClip), farClip(farClip) {
    forwardDirection = glm::vec3(0, 0, -1);
    position = glm::vec3(0, 0, 6);

    calculateView();
    calculateViewportTransform();
}

bool Camera::onUpdate(float ts) {
    glm::vec2 mousePos = Input::getMousePosition();
    glm::vec2 delta = (mousePos - lastMousePosition) * 0.002f;
    lastMousePosition = mousePos;

    if (!Input::isMouseButtonDown(MouseButton::Right)) {
        Input::setCursorMode(CursorMode::Normal);
        return false;
    }

    Input::setCursorMode(CursorMode::Locked);

    bool moved = false;

    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 rightDirection = glm::cross(forwardDirection, upDirection);

    float speed = 5.0f;

    // movement
    if (Input::isKeyDown(KeyCode::W)) {
        position += forwardDirection * speed * ts;
        moved = true;
    }
    else if (Input::isKeyDown(KeyCode::S)) {
        position -= forwardDirection * speed * ts;
        moved = true;
    }
    if (Input::isKeyDown(KeyCode::A)) {
        position -= rightDirection * speed * ts;
        moved = true;
    }
    else if (Input::isKeyDown(KeyCode::D)) {
        position += rightDirection * speed * ts;
        moved = true;
    }
    if (Input::isKeyDown(KeyCode::Q)) {
        position -= upDirection * speed * ts;
        moved = true;
    }
    else if (Input::isKeyDown(KeyCode::E)) {
        position += upDirection * speed * ts;
        moved = true;
    }

    // Rotation
    if (delta.x != 0.0f || delta.y != 0.0f) {
        float pitchDelta = delta.y * getRotationSpeed();
        float yawDelta = delta.x * getRotationSpeed();

        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection), glm::angleAxis(-yawDelta, upDirection)));
        forwardDirection = glm::rotate(q, forwardDirection);

        moved = true;
    }

    if (moved) {
        calculateView();
        calculateProjection();
        calculateRayDirections();
    }

    return moved;
}

void Camera::onResize(uint32_t width, uint32_t height) {
    if (width == viewportWidth && height == viewportHeight) {
        return;
    }

    viewportWidth = width;
    viewportHeight = height;

    calculateProjection();
    calculateRayDirections();
    calculateViewportTransform();
}

float Camera::getRotationSpeed() {
    return 0.3f;
}

void Camera::calculateViewportTransform() {
    // glm::mat4 constructor is column-first, which means it looks like transpose
    viewportTransform = glm::mat4{
        (float)viewportWidth / 2, 0, 0, 0,
        0, (float)viewportHeight / 2, 0, 0,
        0, 0, 1, 0,
        (float)viewportWidth / 2, (float)viewportHeight / 2, 0, 1
    };
}

void Camera::calculateProjection() {
    projection = glm::perspectiveFov(glm::radians(verticalFOV), (float)viewportWidth, (float)viewportHeight, nearClip, farClip);
    inverseProjection = glm::inverse(projection);
}

void Camera::calculateView() {
    view = glm::lookAt(position, position + forwardDirection, glm::vec3(0, 1, 0));
    inverseView = glm::inverse(view);
}

void Camera::calculateRayDirections() {
    rayDirections.resize(viewportWidth * viewportHeight);
    for (uint32_t y = 0; y < viewportHeight; ++y) {
        for (uint32_t x = 0; x < viewportWidth; ++x) {
            glm::vec2 coord = { (float)x / (float)viewportWidth, (float)y / (float)viewportHeight };
            coord = coord * 2.0f - 1.0f; // [-1, 1]

            glm::vec4 target = inverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
            glm::vec3 rayDirection = glm::vec3(inverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0));
            rayDirections[x + y * viewportWidth] = rayDirection;
        }
    }
}
