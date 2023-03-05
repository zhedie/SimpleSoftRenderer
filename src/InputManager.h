#pragma once
#include <vector>
// #include "Camera.h"
#include "SurroundCamera.h"

class InputManager {

public:
    // mouse
    bool mouse_pos_valid;
    float mouse_pos_x;
    float mouse_pos_y;
    float mouse_delta_x;
    float mouse_delta_y;
    bool mouse_left_button;
    bool mouse_right_button;
    float mouse_wheel;

    //keyboard
    std::vector<bool> keys;
public:
    InputManager();

    void reset();

    void get_inputs();

    // void process_input_for_camera(Camera *camera);   // used for a fps camera
    void process_input_for_surround_camera(SurroundCamera *camera);
};