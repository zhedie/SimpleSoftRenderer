#pragma once

class Config {
public:
    float camera_surround_point_y;

    float model_scale;

    Config() {
        camera_surround_point_y = 0;
        model_scale = 1;
    }
};