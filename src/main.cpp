#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "global.h"
#include "GUI.h"
#include "SurroundCamera.h"
#include "InputManager.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "ArgParser.hpp"

#ifdef _MSC_VER
#pragma warning (disable: 4819)
#endif

int main(int argc, char* argv[]) {
    ArgParser argparser;
    argparser.add_argument("model_path", "The path of model to be rendered", "string");
    argparser.parse(argc, argv);
    std::string model_path = argparser.get_argument<std::string>("model_path");

    glfwInit();
    GLFWwindow *window = create_window(WIDTH, HEIGHT, "Renderer");
    if (window == NULL) {
        return -1;
    }

    GUI gui(window);
    SurroundCamera camera(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 10), 45, float(WIDTH) / HEIGHT, -0.1f, -50.f);
    InputManager input_manager;

    Model model(model_path);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    GLuint frame = -1;
    VertexShader vs;
    FragmentShader fs;

    FrameBuf frame_buf(WIDTH, HEIGHT);
    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        input_manager.get_inputs();
        input_manager.process_input_for_surround_camera(&camera);

        vs.set_model(Eigen::Matrix4f::Identity());
        vs.set_view(camera.get_view_matrix());
        vs.set_projection(camera.get_perspective_projection_matrix());
        fs.set_eye_pos(camera.get_position());

        model.render(&frame_buf, vs, fs);
        frame_buf.gen_tex(&frame);


        gui.window();
        gui.set_frame(frame);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        gui.render();
        frame_buf.reset();
        glDeleteTextures(1, &frame);
        glfwSwapBuffers(window);
    }

    // clean
    gui.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}