#pragma once
#include <random>

#include <glm/glm.hpp>

class Random {
    static thread_local std::mt19937 randomEngine;
    static std::uniform_int_distribution<std::mt19937::result_type> distribution;
public:
    // disable constructor
    Random() = delete;

    static void init() {
        randomEngine.seed(std::random_device()());
    }

    static uint32_t UInt() {
        return distribution(randomEngine);
    }

    // return value [_min, _max]
    static uint32_t UInt(uint32_t _min, uint32_t _max) {
        return distribution(randomEngine) % (_max - _min + 1) + _min;
    }

    // return decimal
    static float Float() {
        return (float)distribution(randomEngine) / (float)std::numeric_limits<uint32_t>::max();
    }

    static glm::vec3 Vec3() {
        return glm::vec3(Float(), Float(), Float());
    }

    static glm::vec3 Vec3(float minVal, float maxVal) {
        return glm::vec3(Float() * (maxVal - minVal) + minVal, Float() * (maxVal - minVal) + minVal, Float() * (maxVal - minVal) + minVal);
    }

    static glm::vec3 unitVec3() {
        return glm::normalize(Vec3() - 0.5f);
    }
};