#include "Shader.h"

Eigen::Vector4f Texture::get_value(float u, float v) {
    int index = static_cast<int>(width * u) + static_cast<int>(v * height - 1) * width;
    if (index < 0 || index > width * height) {
        return Eigen::Vector4f(0.f, 0.f, 0.f, 0.f);
    }
    return Eigen::Vector4f(data[4 * index], data[4 * index + 1], data[4 * index + 2], data[4 * index + 3]);
}

VertexShader::VertexShader() {
    Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();
    model = identity;
    view = identity;
    projection = identity;
}

VertexShader::VertexShader(Eigen::Matrix4f model, Eigen::Matrix4f view, Eigen::Matrix4f projection)
    : model(model), view(view), projection(projection) {

}

void VertexShader::set_model(Eigen::Matrix4f model) {
    this->model = model;
}

void VertexShader::set_view(Eigen::Matrix4f view) {
    this->view = view;
}

void VertexShader::set_projection(Eigen::Matrix4f projection) {
    this->projection = projection;
}

Eigen::Matrix4f VertexShader::get_model() const {
    return this->model;
}

Eigen::Matrix4f VertexShader::get_view() const {
    return this->view;
}

Eigen::Matrix4f VertexShader::get_projection() const {
    return this->projection;
}

V2F VertexShader::vertex_shader(const Vertex &vertex) {
    V2F v2f;
    v2f.position = projection * view * model * vertex.position;
    v2f.color = vertex.color;
    v2f.normal = vertex.normal;
    v2f.texcoords = vertex.texcoords;
    return v2f;
}

FragmentShader::FragmentShader() {
    diffuse_tex = nullptr;
    specular_tex = nullptr;
    normal_tex = nullptr;
    height_tex = nullptr;
}

Eigen::Vector4f FragmentShader::fragment_shader(const V2F &vertex) {
    Eigen::Vector4f res = { 0, 0, 0, 0 };
    Eigen::Vector4f value;
    Eigen::Vector3f diffuse_color;
    Eigen::Vector3f specular_color;
    if (diffuse_tex != nullptr) {
        value = diffuse_tex->get_value(vertex.texcoords[0], vertex.texcoords[1]);
        diffuse_color << value.x(), value.y(), value.z();
    }
    if (specular_tex != nullptr) {
        value = specular_tex->get_value(vertex.texcoords[0], vertex.texcoords[1]);
        specular_color << value.x(), value.y(), value.z();
    }


    Eigen::Vector3f ka = ka_default;
    Eigen::Vector3f kd = (diffuse_tex != nullptr) ? diffuse_color / 255.f : kd_default;
    Eigen::Vector3f ks = (specular_tex != nullptr) ? specular_color / 255.f : ks_default;

    // auto l1 = Light{{-20, 20, 20}, {500, 500, 500}};
    auto l2 = Light{{-20, 20, 0}, {500, 500, 500}};

    // std::vector<Light> lights = {l1, l2};
    std::vector<Light> lights = {l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};

    float p = 150;

    Eigen::Vector3f color = (diffuse_tex != nullptr) ? diffuse_color : vertex.color;
    Eigen::Vector3f point = vertex.viewspace_pos;
    Eigen::Vector3f normal = vertex.normal;

    float kh = 0.2f, kn = 0.1f;
    
    if (height_tex != nullptr || normal_tex != nullptr) {
        auto &tex = (height_tex != nullptr ? height_tex : normal_tex);
        const int w = tex->width;
        const int h = tex->height;
        auto f = [&](const float u, const float v) {
            return tex->get_value(u, v).norm();
        };
        const float u = vertex.texcoords[0], v = vertex.texcoords[1];

        auto n = normal;
        float x = n[0], y = n[1], z = n[2];
        Eigen::Vector3f t(x * y / std::sqrt(x * x + z * z), std::sqrt(x * x + z * z), z * y / sqrt(x * x + z * z));
        Eigen::Vector3f b = n.cross(t);
        Eigen::Matrix3f TBN;
        TBN << t, b, n;
        float dU = kh * kn * (f(u + 1.0f / w, v) - f(u, v));
        float dV = kh * kn * (f(u, v + 1.0f / h) - f(u, v));
        Eigen::Vector3f ln(-dU, -dV, 1);

        if (height_tex != nullptr) {
            point += kn * n * f(u, v);
        }
        normal = (TBN * ln).normalized();
    }

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        Eigen::Vector3f light_dir = (light.position - point).normalized();
        float distance = (light.position - point).norm();
        Eigen::Vector3f eye_dir = (eye_pos - point).normalized();
        Eigen::Vector3f halfvec = (eye_dir + light_dir).normalized();
        

        Eigen::Vector3f ambient = ka.cwiseProduct(amb_light_intensity);
        Eigen::Vector3f diffuse = kd.cwiseProduct(light.intensity) / (distance * distance) * std::max(0.0, static_cast<double>(light_dir.dot(normal)));
        Eigen::Vector3f specular = ks.cwiseProduct(light.intensity) / (distance * distance) * std::pow(std::max(0.0, static_cast<double>(halfvec.dot(normal))), p);

        result_color += ambient + diffuse + specular;
    }

    result_color *= 255.f;
    res << result_color.x(), result_color.y(), result_color.z(), 255.f;
    return res;
}

void FragmentShader::set_eye_pos(Eigen::Vector3f eye_pos) {
    this->eye_pos = eye_pos;
}

void FragmentShader::set_diffuse_texture(Texture *texture) {
    this->diffuse_tex = texture;
}

void FragmentShader::set_specular_texture(Texture *texture) {
    this->specular_tex = texture;
}

void FragmentShader::set_normal_texture(Texture *texture) {
    this->normal_tex = texture;
}

void FragmentShader::set_height_texture(Texture *texture) {
    this->height_tex = texture;
}

void FragmentShader::set_ka(Eigen::Vector3f ka) {
    ka_default = ka;
}
void FragmentShader::set_kd(Eigen::Vector3f kd) {
    kd_default = kd;
}
void FragmentShader::set_ks(Eigen::Vector3f ks) {
    ks_default = ks;
}
