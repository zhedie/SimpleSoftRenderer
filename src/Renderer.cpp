#include "Renderer.h"

void setTexture(FragmentShader &fs, Texture &tex) {
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
        // fs.set_height_texture(&tex);
    }
}

void counterclockwise(std::array<V2F, 3>& v) {
    float ab_x = v[1].position[0] - v[0].position[0];
    float ab_y = v[1].position[1] - v[0].position[1];
    float bc_x = v[2].position[0] - v[1].position[0];
    float bc_y = v[2].position[1] - v[1].position[1];
    float ca_x = v[0].position[0] - v[2].position[0];
    float ca_y = v[0].position[1] - v[2].position[1];
    if ((ca_x * ab_y - ca_y * ab_x) < 0) {
        // not counterclockwise
        std::swap(v[1], v[2]);
        ab_x = v[1].position[0] - v[0].position[0];
        ab_y = v[1].position[1] - v[0].position[1];
        bc_x = v[2].position[0] - v[1].position[0];
        bc_y = v[2].position[1] - v[1].position[1];
        ca_x = v[0].position[0] - v[2].position[0];
        ca_y = v[0].position[1] - v[2].position[1];
    }
}

static std::tuple<float, float, float> compute_barycentric2D(std::array<Eigen::Vector4f, 3> v, float x, float y) {
    float alpha = (x * (v[1][1] - v[2][1]) + (v[2][0] - v[1][0]) * y + v[1][0] * v[2][1] - v[2][0] * v[1][1]) / (v[0][0] * (v[1][1] - v[2][1]) + (v[2][0] - v[1][0]) * v[0][1] + v[1][0] * v[2][1] - v[2][0] * v[1][1]);
    float beta = (x *(v[2][1] - v[0][1]) + (v[0][0] - v[2][0]) * y + v[2][0] * v[0][1] - v[0][0] * v[2][1]) / (v[1][0] * (v[2][1] - v[0][1]) + (v[0][0] - v[2][0]) * v[1][1] + v[2][0] * v[0][1] - v[0][0] * v[2][1]);
    float gamma = (x *(v[0][1] - v[1][1]) + (v[1][0] - v[0][0]) * y + v[0][0] * v[1][1] - v[1][0] * v[0][1]) / (v[2][0] * (v[0][1] - v[1][1]) + (v[1][0] - v[0][0]) * v[2][1] + v[0][0] * v[1][1] - v[1][0] * v[0][1]);
    return {alpha, beta, gamma};
}

template<typename T>
static T lerp(double alpha, double beta, double gamma, T x, T y, T z) {
    return alpha * x + beta * y + gamma * z;
}

void Renderer::rasterize(std::array<V2F, 3> &v, FragmentShader &fs) {
    int xmin = std::max((int)floor(std::min(std::min(v[0].position[0], v[1].position[0]), v[2].position[0])), 0);
    int ymin = std::max((int)floor(std::min(std::min(v[0].position[1], v[1].position[1]), v[2].position[1])), 0);
    int xmax = std::min((int)ceil(std::max(std::max(v[0].position[0], v[1].position[0]), v[2].position[0])), static_cast<int>(frame_buffer.width - 1));
    int ymax = std::min((int)ceil(std::max(std::max(v[0].position[1], v[1].position[1]), v[2].position[1])), static_cast<int>(frame_buffer.height - 1));

    counterclockwise(v);
    float ab_x = v[1].position[0] - v[0].position[0];
    float ab_y = v[1].position[1] - v[0].position[1];
    float bc_x = v[2].position[0] - v[1].position[0];
    float bc_y = v[2].position[1] - v[1].position[1];
    float ca_x = v[0].position[0] - v[2].position[0];
    float ca_y = v[0].position[1] - v[2].position[1];
    float C1 = v[0].position[0] * v[1].position[1] - v[1].position[0] * v[0].position[1];
    float C2 = v[1].position[0] * v[2].position[1] - v[2].position[0] * v[1].position[1];
    float C3 = v[2].position[0] * v[0].position[1] - v[0].position[0] * v[2].position[1];
    for (int j = ymin; j < ymax; ++j) {
        double pa = ab_x * (j + 0.5) - ab_y * (xmin + 0.5) + C1;
        double pb = bc_x * (j + 0.5) - bc_y * (xmin + 0.5) + C2;
        double pc = ca_x * (j + 0.5) - ca_y * (xmin + 0.5) + C3;
        bool has_entered = false;
        for (int i = xmin; i < xmax; ++i) {
            if (pa > 0 && pb > 0 && pc > 0) {
                float x = i + 0.5f, y = j + 0.5f;
                std::array<Eigen::Vector4f, 3> vp = {v[0].position, v[1].position, v[2].position};
                auto [a, b, c] = compute_barycentric2D(vp, x, y);
                // perspective projection correction 
                float alpha = a / v[0].position[3];
                float beta = b / v[1].position[3];
                float gamma = c / v[2].position[3];
                float z_correction = 1.0f / (alpha + beta + gamma);
                
                // depth test
                int index = depth_buffer.get_index(i, j);
                if (z_correction > depth_buffer.buf[index]) { // depth is negative
                    depth_buffer.buf[index] = z_correction;

                    // lerp
                    Eigen::Vector3f color = z_correction * lerp(alpha, beta, gamma, v[0].color, v[1].color, v[2].color);
                    Eigen::Vector2f texcoords = z_correction * lerp(alpha, beta, gamma, v[0].texcoords, v[1].texcoords, v[2].texcoords);
                    Eigen::Vector3f normal = z_correction * lerp(alpha, beta, gamma, v[0].normal, v[1].normal, v[2].normal);
                    Eigen::Vector3f view_pos = z_correction * lerp(alpha, beta, gamma, v[0].viewspace_pos, v[1].viewspace_pos, v[2].viewspace_pos);
                    V2F v2f;
                    v2f.color = color;
                    v2f.texcoords = texcoords;
                    v2f.normal = normal;
                    v2f.viewspace_pos = view_pos;

                    // R G B A
                    color_buffer.buf[index] = fs.fragment_shader(v2f);
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

void Renderer::render(Mesh &mesh, VertexShader &vs, FragmentShader &fs) {
    // vs
    std::vector<V2F> v2fs;
    for (const auto &i : mesh.vertices) {
        v2fs.emplace_back(vs.vertex_shader(i));
    }
    // calculate viewspace position
    Eigen::Matrix4f inverse = (vs.get_projection() * vs.get_view()).inverse();
    for (auto &i : v2fs) {
        auto pos = inverse * i.position;
        i.viewspace_pos = Eigen::Vector3f(pos.x(), pos.y(), pos.z());
    }
    
    // set fs
    fs.reset_texture();
    for (auto &tex : mesh.textures) {
        setTexture(fs, tex);
    }
    fs.set_ka(mesh.ka);
    fs.set_kd(mesh.kd);
    fs.set_ks(mesh.ks);
    fs.set_Ns(mesh.Ns);

    Eigen::Matrix4f viewport = Eigen::Matrix4f::Identity();
    viewport << static_cast<float>(frame_buffer.width) / 2, 0, 0, static_cast<float>(frame_buffer.width) / 2,
                0, static_cast<float>(frame_buffer.height) / 2, 0, static_cast<float>(frame_buffer.height) / 2,
                0, 0, 1, 0,
                0, 0, 0, 1;
    for (size_t i = 0, size = mesh.indices.size(); i < size; i += 3) {
        // primitives assembly
        std::array<V2F, 3> v {v2fs[mesh.indices[i]], v2fs[mesh.indices[i + 1]], v2fs[mesh.indices[i + 2]]};
        
        // view port transformation
        for (int j = 0; j < 3; ++j) {
            float w = v[j].position[3];
            v[j].position /= w;

            v[j].position = viewport * v[j].position;
            v[j].position[3] = w;
        }

        rasterize(v, fs);
    }
}

void Renderer::render(Model &model, VertexShader &vs, FragmentShader &fs) {
    for (auto &mesh : model.meshes) {
        render(mesh, vs, fs);
    }
}

void Renderer::reset_buffer() {
    color_buffer.reset(Eigen::Vector4f(0.f, 0.f, 0.f, 0.f));
    depth_buffer.reset(std::numeric_limits<float>::lowest());
    frame_buffer.reset();
}

Renderer::Renderer(int width, int height) : frame_buffer(width, height), color_buffer(width, height), depth_buffer(width, height) {

}

void Renderer::generate_frame() {
    const size_t len = color_buffer.buf.size();
    for (size_t i = 0; i < len; ++i) {
        frame_buffer.buf[4 * i] = static_cast<unsigned char>(color_buffer.buf[i].x());
        frame_buffer.buf[4 * i + 1] = static_cast<unsigned char>(color_buffer.buf[i].y());
        frame_buffer.buf[4 * i + 2] = static_cast<unsigned char>(color_buffer.buf[i].z());
        frame_buffer.buf[4 * i + 3] = static_cast<unsigned char>(color_buffer.buf[i].w());
        // frame_buffer.buf[4 * i + 3] = 0xff;
    }
}

void Renderer::convert_frame_to_tex(GLuint *tex) {
    GLuint frame_tex;
    glGenTextures(1, &frame_tex);
    glBindTexture(GL_TEXTURE_2D, frame_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_buffer.width, frame_buffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &frame_buffer.buf[0]);
    *tex = frame_tex; 
}
