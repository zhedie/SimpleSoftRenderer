#pragma once
#include <Eigen/Dense>

class SurroundCamera {
    Eigen::Vector3f surround_point;
    Eigen::Vector3f position;
    float theta;    // angle between position and Y-axis
    float phi;      // angle between position and Z-axis
    float distance;

    Eigen::Vector3f front;
    Eigen::Vector3f right;
    Eigen::Vector3f up;

    float fov; // angle, not radius
    float aspect;
    // z_near and z_far are negative
    float znear;
    float zfar;

    void update_camera();
public:
    SurroundCamera(Eigen::Vector3f _surround_point, Eigen::Vector3f _position, float _fov, float _aspect, float _znear, float _zfar);
    SurroundCamera(Eigen::Vector3f _surround_point, float _theta_angle, float _phi_angle, float _distance, float _fov, float _aspect, float _znear, float _zfar);

    Eigen::Matrix4f get_view_matrix();
    Eigen::Matrix4f get_perspective_projection_matrix();

    Eigen::Vector3f get_position();

    void move_camera(float delta_phi, float delta_theta);
    void modify_fov(float delta_fov);
};