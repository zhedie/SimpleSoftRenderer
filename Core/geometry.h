#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Cube {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    Cube() {
        vertices = {
            {-0.1f, 0.1f, 0.1f},
            {-0.1f, -0.1f, 0.1f},
            {0.1f, -0.1f, 0.1f},
            {0.1f, 0.1f, 0.1f},
            {0.1f, -0.1f, -0.1f},
            {0.1f, 0.1f, -0.1f},
            {-0.1f, -0.1f, -0.1f},
            {-0.1f, 0.1f, -0.1f},
        };
        indices = {
            0, 1, 3,
            1, 2, 3,
            2, 5, 3,
            2, 4, 5,
            4, 7, 5,
            4, 6, 7,
            0, 7, 6,
            0, 6, 1,
            0, 3, 5,
            0, 5, 7,
            1, 4, 2,
            1, 6, 4
        };
    }
};