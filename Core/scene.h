#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "model.h"

struct DirectionLight {
    glm::vec3 direction;
    glm::vec3 intensity;
};

class Scene {
public:
    std::vector<Model> models;
    std::vector<DirectionLight> lights;
    glm::vec3 skyColor{0.6f, 0.7f, 0.8f};
};