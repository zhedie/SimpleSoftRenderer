#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <string>

#include "global.h"

GLFWwindow* create_window(int width, int height, std::string title);

class GUI {
public:
    GUI(GLFWwindow *window);
    void destroy();
    void window();
    void set_frame(GLuint frame);
    void render();
};