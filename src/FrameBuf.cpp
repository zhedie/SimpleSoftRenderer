#include "FrameBuf.h"

FrameBuf::FrameBuf(int width, int height) 
    : width(width), height(height) {
    frame_buf.resize(width * height * 4, 0);
    depth_buf.resize(width * height, std::numeric_limits<float>::lowest());
}

void FrameBuf::reset() {
    std::fill(frame_buf.begin(), frame_buf.end(), 0);
    std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::lowest());
}

int FrameBuf::get_index(int x, int y) {
    return (height - y - 1) * width + x;
}

void FrameBuf::gen_tex(GLuint *frame) {
    GLuint frame_tex;
    glGenTextures(1, &frame_tex);
    glBindTexture(GL_TEXTURE_2D, frame_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &frame_buf[0]);
    *frame = frame_tex; 
}