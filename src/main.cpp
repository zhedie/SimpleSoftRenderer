#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <Eigen/Geometry>

#include "global.h"
#include "GUI.h"
#include "SurroundCamera.h"
#include "InputManager.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "ArgParser.hpp"
#include "Renderer.h"
#include "Config.h"

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

    InputManager input_manager;
    Config config;
    GUI gui(window, &config);
    SurroundCamera camera(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 10), 45, float(WIDTH) / HEIGHT, -0.1f, -50.f);

    // ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

    Model model(model_path);

    GLuint frame = -1;
    VertexShader vs;
    FragmentShader fs;
    Renderer renderer(WIDTH, HEIGHT);
    // set light
    PointLight light;
    light.position = Eigen::Vector3f(0, 3, 3);
    light.ambient = Eigen::Vector3f(0.2f, 0.2f, 0.2f);
    light.diffuse = Eigen::Vector3f(0.5f, 0.5f, 0.5f);
    light.specular = Eigen::Vector3f(1.f, 1.f, 1.f);
    light.constant = 1.0f;
    light.linear = 0.09f;
    light.quadratic = 0.032f;
    fs.add_point_light(light);
    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        input_manager.get_inputs();
        input_manager.process_input_for_surround_camera(&camera);

        camera.set_surround_point(Eigen::Vector3f(0, config.camera_surround_point_y, 0));

        Eigen::Transform<float, 3, Eigen::Affine> t;
        t = Eigen::Scaling(config.model_scale);
        vs.set_model(t * Eigen::Matrix4f::Identity());
        vs.set_view(camera.get_view_matrix());
        vs.set_projection(camera.get_perspective_projection_matrix());
        fs.set_eye_pos(camera.get_position());

        renderer.reset_buffer();
        renderer.render(model, vs, fs);
        renderer.generate_frame();
        renderer.convert_frame_to_tex(&frame);


        gui.window();
        gui.show_settings_window();
        gui.set_frame(frame);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        gui.render();
        // frame_buf.reset();
        glDeleteTextures(1, &frame);
        glfwSwapBuffers(window);
    }

    // clean
    gui.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}