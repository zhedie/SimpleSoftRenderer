#include "InputManager.h"
#include "imgui.h"
#include <iostream>

InputManager::InputManager() {
    keys.resize(ImGuiKey_COUNT, false);
}

void InputManager::reset() {

}

void InputManager::get_inputs() {
    ImGuiIO &io = ImGui::GetIO();
    mouse_pos_valid = ImGui::IsMousePosValid();
    if (mouse_pos_valid) {
        mouse_pos_x = io.MousePos.x;
        mouse_pos_y = io.MousePos.y;
        mouse_delta_x = io.MouseDelta.x;
        mouse_delta_y = io.MouseDelta.y;
        mouse_left_button = ImGui::IsMouseDown(0);
        mouse_right_button = ImGui::IsMouseDown(1);
        mouse_wheel = io.MouseWheel;    // imgui background cannot receive io.MouseWheel
    }
    for (ImGuiKey key = ImGuiKey_KeysData_OFFSET; key < ImGuiKey_COUNT; key = (ImGuiKey)(key + 1)) {
        keys[key] = ImGui::IsKeyDown(key);
    }
}

// used for a fps camera
// void InputManager::process_input_for_camera(Camera *camera) {
//     if (keys[ImGuiKey_W]) {
//         camera->move_camera(CameraMovement::FORWARD, 1);
//         std::cout << "c" << std::endl;
//     }
//     if (keys[ImGuiKey_S]) {
//         camera->move_camera(CameraMovement::BACKWARD, 0.05);
//     }
//     if (keys[ImGuiKey_A]) {
//         camera->move_camera(CameraMovement::LEFT, 0.05);
//     }
//     if (keys[ImGuiKey_D]) {
//         camera->move_camera(CameraMovement::RIGHT, 0.05);
//     }

//     if (mouse_pos_valid) {
//         if (mouse_right_button && (mouse_delta_x != 0 || mouse_delta_y != 0)) {
//             camera->rotate_camera(mouse_delta_x, mouse_delta_y);
//         }
//     }

// }

void InputManager::process_input_for_surround_camera(SurroundCamera *camera) {
    if (mouse_pos_valid) {
        if (mouse_right_button && (mouse_delta_x != 0 || mouse_delta_y != 0)) {
            camera->move_camera(-mouse_delta_x * 0.1f, -mouse_delta_y * 0.1f);
        }
        if (keys[ImGuiKey_W]) {
            camera->modify_fov(-0.1);
        }
        if (keys[ImGuiKey_S]) {
            camera->modify_fov(0.1);
        }
    }
}