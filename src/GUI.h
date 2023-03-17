#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <string>

#include "global.h"
#include "Config.h"

GLFWwindow* create_window(int width, int height, std::string title);

class GUI {
    Config *config;
public:
    GUI(GLFWwindow *window, Config *config);
    void destroy();
    void window();
    void show_settings_window();
    void set_frame(GLuint frame);
    void render();
};