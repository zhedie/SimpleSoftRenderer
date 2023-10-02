#pragma once
#include <iostream>
#include <glm/glm.hpp>

namespace Utils {
    inline uint32_t glmVec4ToUint32t(glm::vec4 color) {
        uint32_t r = (uint32_t)(color.r * 255.0f);
        uint32_t g = (uint32_t)(color.g * 255.0f);
        uint32_t b = (uint32_t)(color.b * 255.0f);
        uint32_t a = (uint32_t)(color.a * 255.0f);

        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        return result;
    }

    template<typename T>
    inline T lerp(float alpha, float beta, T x, T y, T z) {
        return alpha * x + beta * y + (1 - alpha - beta) * z;
    }

    template<typename T>
    inline T lerp(float alpha, float beta, float gamma, T x, T y, T z) {
        return alpha * x + beta * y + gamma * z;
    }

    inline void printGLMMat4(const glm::mat4 &mat) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << mat[i][j] << ' ';
            }
            std::cout << std::endl;
        }
    }

    inline glm::mat4 calcScaleTranslateMat4(const glm::vec3 &scale, const glm::vec3 &translate) {
        glm::mat4 mat {
            scale.x, 0, 0, 0,
            0, scale.y, 0, 0,
            0, 0, scale.z, 0,
            translate.x, translate.y, translate.z, 1
        };
        return mat;
    }
}