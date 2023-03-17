#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <iostream>

struct Vertex {
    Eigen::Vector4f position;
    Eigen::Vector3f color;
    Eigen::Vector3f normal;
    Eigen::Vector2f texcoords;
    Eigen::Vector3f tangent;
    Eigen::Vector3f Bitangent;
};

struct Texture {
    std::vector<unsigned char> data;
    int width;
    int height;
    std::string type;
    std::string path;

    Eigen::Vector4f get_value(float u, float v);
};

struct V2F {
    Eigen::Vector4f position;
    Eigen::Vector3f color;
    Eigen::Vector3f normal;
    Eigen::Vector2f texcoords;
    Eigen::Vector3f viewspace_pos;
};

class VertexShader {
    Eigen::Matrix4f model;
    Eigen::Matrix4f view;
    Eigen::Matrix4f projection;
public:
    VertexShader();
    VertexShader(Eigen::Matrix4f model, Eigen::Matrix4f view, Eigen::Matrix4f projection);

    void set_model(Eigen::Matrix4f model);
    void set_view(Eigen::Matrix4f view);
    void set_projection(Eigen::Matrix4f projection);

    Eigen::Matrix4f get_model() const;
    Eigen::Matrix4f get_view() const;
    Eigen::Matrix4f get_projection() const;

    virtual V2F vertex_shader(const Vertex &vertex);
};

struct PointLight {
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;

    Eigen::Vector3f ambient;
    Eigen::Vector3f diffuse;
    Eigen::Vector3f specular;

    float constant;
    float linear;
    float quadratic;
};

class FragmentShader {
    Eigen::Vector3f eye_pos;
    Texture *diffuse_tex;
    Texture *specular_tex;
    Texture *normal_tex;
    Texture *height_tex;

    Eigen::Vector3f ka_default;
    Eigen::Vector3f kd_default;
    Eigen::Vector3f ks_default;
    float Ns;
    std::vector<PointLight> lights;
public:
    FragmentShader();

    virtual Eigen::Vector4f fragment_shader(const V2F &vertex);

    void set_eye_pos(Eigen::Vector3f eye_pos);

    void set_diffuse_texture(Texture *texture);
    void set_specular_texture(Texture *texture);
    void set_normal_texture(Texture *texture);
    void set_height_texture(Texture *texture);

    void set_ka(Eigen::Vector3f ka);
    void set_kd(Eigen::Vector3f kd);
    void set_ks(Eigen::Vector3f ks);
    void set_Ns(float Ns);

    void reset_texture();

    int add_point_light(PointLight light);
};