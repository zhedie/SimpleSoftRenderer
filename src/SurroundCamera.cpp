#define _USE_MATH_DEFINES
#include "SurroundCamera.h"
#include <cmath>

const static Eigen::Vector3f world_up(0, 1, 0);

static float convert_to_0_360(float angle) {
    while (angle > 360) {
        angle -= 360;
    }
    while (angle < -360) {
        angle += 360;
    }
    return angle;
}

static inline float radian(float angle) {
    return static_cast<float>(angle / 180 * M_PI);
}

void SurroundCamera::update_camera() {
    float theta_radius = radian(theta);
    float phi_radius = radian(phi);
    position.x() = distance * std::sin(theta_radius) * std::sin(phi_radius) + surround_point.x();
    position.y() = distance * std::cos(theta_radius) + surround_point.y();
    position.z() = distance * std::sin(theta_radius) * std::cos(phi_radius) + surround_point.z();

    Eigen::Vector3f towards = surround_point - position;
    front = towards.normalized();
    right = front.cross(world_up).normalized();
    up = right.cross(front).normalized();
}

SurroundCamera::SurroundCamera(Eigen::Vector3f _surround_point, Eigen::Vector3f _position, float _fov, float _aspect, float _znear, float _zfar)
    : surround_point(_surround_point), position(_position), fov(_fov), aspect(_aspect), znear(_znear), zfar(_zfar) {
    Eigen::Vector3f towards = surround_point - position;
    distance = towards.norm();
    theta = static_cast<float>(std::acos((-towards.y()) / distance) / M_PI * 180);
    phi = static_cast<float>(std::acos((-towards.z()) / (distance * std::sin(radian(theta))))  / M_PI * 180);
    if (towards.x() > 0) {
        phi = -phi;
    }
    if (theta < 5) {
        theta = 5;
    }
    else if (theta > 175) {
        theta = 175;
    }
    update_camera();

}

SurroundCamera::SurroundCamera(Eigen::Vector3f _surround_point, float _theta_angle, float _phi_angle, float _distance, float _fov, float _aspect, float _znear, float _zfar) 
    : surround_point(_surround_point), distance(_distance), fov(_fov), aspect(_aspect), znear(_znear), zfar(_zfar) {
    theta = _theta_angle;
    phi = _phi_angle;
    if (theta > 180) {
        phi += 180;
        phi = convert_to_0_360(phi);
        theta = 360 - theta;
    }
    if (theta < 5) {
        theta = 5;
    }
    else if (theta > 175) {
        theta = 175;
    }
    update_camera();
}

Eigen::Matrix4f SurroundCamera::get_view_matrix() {
    // Position(Px, Py, Pz), Up(Ux, Uy, Uz), Front(Fx, Fy, Fz), Right(Rx, Ry, Rz)
    // M_translate = (1, 0, 0, -Px)
    //               (0, 1, 0, -Py)
    //               (0, 0, 1, -Pz)
    //               (0, 0, 0,   1)
    // M_rotate = ( Rx,  Ry,  Rz, 0)
    //            ( Ux,  Uy,  Uz, 0)
    //            (-Fx, -Fy, -Fz, 0)
    //            (  0,   0,   0, 1)
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view << right[0], right[1], right[2], -(position.dot(right)),
            up[0], up[1], up[2], -(position.dot(up)),
            -front[0], -front[1], -front[2], (position.dot(front)),
            0, 0, 0, 1;

    return view;
}

Eigen::Matrix4f SurroundCamera::get_perspective_projection_matrix() {
    float fov_radius = static_cast<float>(fov / 180 * M_PI);
    float t = std::fabs(znear) * std::tan(fov_radius / 2);
    float r = aspect * t;

    // M_perspective = (n, 0, 0, 0)
    //                 (0, n, 0, 0)
    //                 (0, 0, n + f, -n * f)
    //                 (0, 0, 1, 0)
    // M_ortho = (2 / (r - l), 0, 0, 0)
    //           (0, 2 / (t - b), 0, 0)
    //           (0, 0, 2 / (n - f), 0)
    //           (0, 0, 0, 1)
    Eigen::Matrix4f perspective_projection = Eigen::Matrix4f::Identity();
    perspective_projection << znear / r, 0, 0, 0,
                               0, znear / t, 0, 0,
                               0, 0, 2 * (znear + zfar) / (znear - zfar), -znear * zfar / (znear - zfar),
                               0, 0, 1, 0;
    return perspective_projection;
}

Eigen::Vector3f SurroundCamera::get_position() {
    return position;
}

void SurroundCamera::move_camera(float delta_phi, float delta_theta) {
    phi += delta_phi;
    theta += delta_theta;
    if (theta > 175) {
        theta = 175;
    }
    else if (theta < 5) {
        theta = 5;
    }
    phi = convert_to_0_360(phi);


    update_camera();
}

void SurroundCamera::modify_fov(float delta_fov) {
    fov += delta_fov;
    if (fov < 1) {
        fov = 1;
    }
    else if (fov > 45) {
        fov = 45;
    }
}

void SurroundCamera::set_surround_point(Eigen::Vector3f _surround_point) {
    if (this->surround_point == _surround_point) {
        return;
    }
    this->surround_point = _surround_point;
    Eigen::Vector3f towards = surround_point - position;
    distance = towards.norm();
    theta = static_cast<float>(std::acos((-towards.y()) / distance) / M_PI * 180);
    phi = static_cast<float>(std::acos((-towards.z()) / (distance * std::sin(radian(theta))))  / M_PI * 180);
    if (towards.x() > 0) {
        phi = -phi;
    }
    if (theta < 5) {
        theta = 5;
    }
    else if (theta > 175) {
        theta = 175;
    }
    update_camera();
}