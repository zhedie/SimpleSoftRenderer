#pragma once
#include <vector>
#include <GLFW/glfw3.h>

struct FrameBuf {
public:
    int width;
    int height;

    std::vector<unsigned char> frame_buf;
    std::vector<float> depth_buf;


    FrameBuf(int width, int height);

    void reset();

    int get_index(int x, int y);

    void gen_tex(GLuint *frame);
};