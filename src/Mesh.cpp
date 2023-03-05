#include "Mesh.h"

static std::tuple<float, float, float> compute_barycentric2D(std::array<Eigen::Vector4f, 3> v, float x, float y) {
    float alpha = (x * (v[1][1] - v[2][1]) + (v[2][0] - v[1][0]) * y + v[1][0] * v[2][1] - v[2][0] * v[1][1]) / (v[0][0] * (v[1][1] - v[2][1]) + (v[2][0] - v[1][0]) * v[0][1] + v[1][0] * v[2][1] - v[2][0] * v[1][1]);
    float beta = (x *(v[2][1] - v[0][1]) + (v[0][0] - v[2][0]) * y + v[2][0] * v[0][1] - v[0][0] * v[2][1]) / (v[1][0] * (v[2][1] - v[0][1]) + (v[0][0] - v[2][0]) * v[1][1] + v[2][0] * v[0][1] - v[0][0] * v[2][1]);
    float gamma = (x *(v[0][1] - v[1][1]) + (v[1][0] - v[0][0]) * y + v[0][0] * v[1][1] - v[1][0] * v[0][1]) / (v[2][0] * (v[0][1] - v[1][1]) + (v[1][0] - v[0][0]) * v[2][1] + v[0][0] * v[1][1] - v[1][0] * v[0][1]);
    return {alpha, beta, gamma};
}

void Mesh::rasterize_triangles(V2F_Triangle &t, std::array<Eigen::Vector3f, 3> &viewspace_pos, FrameBuf *buf, FragmentShader &fs) {
    std::array<Eigen::Vector4f, 3> v{t.vertices[0].position, t.vertices[1].position, t.vertices[2].position};
    int xmin = std::max(int(floor(std::min(std::min(v[0][0], v[1][0]), v[2][0]))), 0);
    int ymin = std::max(int(floor(std::min(std::min(v[0][1], v[1][1]), v[2][1]))), 0);
    int xmax = std::min(int(ceil(std::max(std::max(v[0][0], v[1][0]), v[2][0]))), static_cast<int>(buf->width - 1));
    int ymax = std::min(int(ceil(std::max(std::max(v[0][1], v[1][1]), v[2][1]))), static_cast<int>(buf->height - 1));

    float ab_x = v[1][0] - v[0][0];
    float ab_y = v[1][1] - v[0][1];
    float bc_x = v[2][0] - v[1][0];
    float bc_y = v[2][1] - v[1][1];
    float ca_x = v[0][0] - v[2][0];
    float ca_y = v[0][1] - v[2][1];

    if ((ca_x * ab_y - ca_y * ab_x) < 0) {
        // not counter-clock
        std::swap(v[1], v[2]);
        std::swap(t.vertices[1], t.vertices[2]);
        ab_x = v[1][0] - v[0][0];
        ab_y = v[1][1] - v[0][1];
        bc_x = v[2][0] - v[1][0];
        bc_y = v[2][1] - v[1][1];
        ca_x = v[0][0] - v[2][0];
        ca_y = v[0][1] - v[2][1];
    }

    float C1 = v[0][0] * v[1][1] - v[1][0] * v[0][1];
    float C2 = v[1][0] * v[2][1] - v[2][0] * v[1][1];
    float C3 = v[2][0] * v[0][1] - v[0][0] * v[2][1];
    for (int j = ymin; j < ymax; ++j) {
        double pa = ab_x * (j + 0.5) - ab_y * (xmin + 0.5) + C1;
        double pb = bc_x * (j + 0.5) - bc_y * (xmin + 0.5) + C2;
        double pc = ca_x * (j + 0.5) - ca_y * (xmin + 0.5) + C3;
        bool has_entered = false;
        for (int i = xmin; i < xmax; ++i) {
            if (pa > 0 && pb > 0 && pc > 0) {
                float x = i + 0.5f, y = j + 0.5f;;
                auto [a, b, c] = compute_barycentric2D(v, x, y);
                // perspective projection correction 
                float alpha = a / v[0][3];
                float beta = b / v[1][3];
                float gamma = c / v[2][3];
                float z_correction = 1.0f / (alpha + beta + gamma);
                
                // depth test
                int index = buf->get_index(i, j);
                if (z_correction > buf->depth_buf[index]) { // depth is negative
                    buf->depth_buf[index] = z_correction;

                    // lerp
                    Eigen::Vector3f color = z_correction * (alpha * t.vertices[0].color + beta * t.vertices[1].color + gamma * t.vertices[2].color);
                    Eigen::Vector2f texcoords = z_correction * (alpha * t.vertices[0].texcoords + beta * t.vertices[1].texcoords + gamma * t.vertices[2].texcoords);
                    Eigen::Vector3f normal = z_correction * (alpha * t.vertices[0].normal + beta * t.vertices[1].normal + gamma * t.vertices[2].normal);
                    Eigen::Vector3f view_pos = z_correction * (alpha * viewspace_pos[0] + beta * viewspace_pos[1] + gamma * viewspace_pos[2]);
                    V2F v2f;
                    v2f.color = color;
                    v2f.texcoords = texcoords;
                    v2f.normal = normal;
                    v2f.viewspace_pos = view_pos;

                    // R G B A
                    Eigen::Vector4f final_color = fs.fragment_shader(v2f);
                    // Eigen::Vector4f final_color(255.f, 255.f, 255.f, 255.f);
                    buf->frame_buf[index * 4] = static_cast<unsigned char>(final_color[0]);
                    buf->frame_buf[index * 4 + 1] = static_cast<unsigned char>(final_color[1]);
                    buf->frame_buf[index * 4 + 2] = static_cast<unsigned char>(final_color[2]);
                    buf->frame_buf[index * 4 + 3] = static_cast<unsigned char>(final_color[3]);
                }
                has_entered = true;
            }
            else if (has_entered) {
                break;
            }
            pa -= ab_y;
            pb -= bc_y;
            pc -= ca_y;
        }
    }
}

Mesh::Mesh() {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) 
    : vertices(vertices), indices(indices), textures(textures) {
    ka = Eigen::Vector3f(0.f, 0.f, 0.f);
    kd = Eigen::Vector3f(0.f, 0.f, 0.f);
    ks = Eigen::Vector3f(0.f, 0.f, 0.f);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Eigen::Vector3f ka, Eigen::Vector3f kd, Eigen::Vector3f ks) 
    : vertices(vertices), indices(indices), textures(textures), ka(ka), kd(kd), ks(ks) {

}

void Mesh::render(FrameBuf *buf, VertexShader &vs, FragmentShader &fs) {
    std::vector<V2F> v2fs;
    for (const auto &i : vertices) {
        v2fs.emplace_back(vs.vertex_shader(i));
    }
    // set texture
    for (auto &tex : textures) {
        if (tex.type == "texture_diffuse") {
            fs.set_diffuse_texture(&tex);
        }
        else if (tex.type == "texture_specular") {
            fs.set_specular_texture(&tex);
        }
        else if (tex.type == "texture_normal") {
            fs.set_normal_texture(&tex);
        }
        else if (tex.type == "texture_height") {
            fs.set_height_texture(&tex);
        }
    }
    fs.set_ka(ka);
    fs.set_kd(kd);
    fs.set_ks(ks);

    Eigen::Matrix4f inverse_projection = vs.get_projection().inverse();

    Eigen::Matrix4f viewport = Eigen::Matrix4f::Identity();
    viewport << static_cast<float>(buf->width) / 2, 0, 0, static_cast<float>(buf->width) / 2,
                0, static_cast<float>(buf->height) / 2, 0, static_cast<float>(buf->height) / 2,
                0, 0, 1, 0,
                0, 0, 0, 1;

    std::vector<V2F_Triangle> triangles;
    for (unsigned i = 0; i < indices.size(); i += 3) {
        V2F_Triangle t{v2fs[indices[i]], v2fs[indices[i + 1]], v2fs[indices[i + 2]]};

        std::array<Eigen::Vector3f, 3> viewspace_pos;
        for (int j = 0; j < 3; ++j) {
            auto pos = inverse_projection * t.vertices[j].position;
            viewspace_pos[j] = Eigen::Vector3f(pos.x(), pos.y(), pos.z());;
        }

        // view port transformation
        for (int j = 0; j < 3; ++j) {
            float w = t.vertices[j].position[3];
            t.vertices[j].position /= w;

            t.vertices[j].position = viewport * t.vertices[j].position;
            t.vertices[j].position[3] = w;
        }
        rasterize_triangles(t, viewspace_pos, buf, fs);
    }
}
