#pragma once
#include "Model.h"
#include "Shader.h"
#include <vector>
#include <GLFW/glfw3.h>

template<typename T>
class Buffer {
public:
    std::vector<T> buf;
    int width;
    int height;
    Buffer(int w, int h) : buf(w * h), width(w), height(h) {
        // std::cout << "buf size = " << buf.size() << std::endl;
    }

    void reset(T value) {
        std::fill(buf.begin(), buf.end(), value);
    }

    int get_index(int i, int j) {
        return (height - j - 1) * width + i;
    }
};

struct sRGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

class FrameBuffer {
public:
    std::vector<unsigned char> buf;
    int width;
    int height;
    void reset() {
        std::fill(buf.begin(), buf.end(), '\0');
    }
    FrameBuffer(int w, int h) : buf(w * h * 4), width(w), height(h) {}
};

class Renderer {
    FrameBuffer frame_buffer;
    Buffer<Eigen::Vector4f> color_buffer;
    Buffer<float> depth_buffer;

    void rasterize(std::array<V2F, 3> &v, FragmentShader &fs);
    void render(Mesh &mesh, VertexShader &vs, FragmentShader &fs);
public:
    Renderer(int width, int height);
    void reset_buffer();
    void render(Model &model, VertexShader &vs, FragmentShader &fs);
    void generate_frame();
    void convert_frame_to_tex(GLuint *tex);
};