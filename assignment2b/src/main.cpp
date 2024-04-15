#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "trimesh.hpp"
#include "shader.hpp"

#define WIN_WIDTH 500
#define WIN_HEIGHT 500
#define FRUSTUM_WIDTH 0.5
#define FRUSTUM_HEIGHT 0.5

class Mat4x4 {
public:

    float m[16];

	Mat4x4(){
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void make_identity(){
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

    void make_scale(float x, float y, float z){
        make_identity();
        m[0] = x; m[5] = y; m[10] = z;
    }

    void make_translation(float x, float y, float z) {
        make_identity();
        m[12] = x;
        m[13] = y;
        m[14] = z;
    }

};

static inline Vec3f operator*(const Mat4x4 &m, const Vec3f &v) {
    Vec3f r(m.m[0] * v[0] + m.m[4] * v[1] + m.m[8] * v[2],
            m.m[1] * v[0] + m.m[5] * v[1] + m.m[9] * v[2],
            m.m[2] * v[0] + m.m[6] * v[1] + m.m[10] * v[2]);
    return r;
}

static Mat4x4 rotation_y(float angle) {
    Mat4x4 rot;
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    rot.m[0] = cosTheta;
    rot.m[2] = -sinTheta;
    rot.m[8] = sinTheta;
    rot.m[10] = cosTheta;
    return rot;
}

static Mat4x4 rotation(const Vec3f &axis, float angle) {
    Mat4x4 rot;
    float cosA = cos(angle);
    float sinA = sin(angle);
    float oneMinusCosA = 1 - cosA;

    rot.m[0] = cosA + axis[0] * axis[0] * oneMinusCosA;
    rot.m[1] = axis[0] * axis[1] * oneMinusCosA - axis[2] * sinA;
    rot.m[2] = axis[0] * axis[2] * oneMinusCosA + axis[1] * sinA;
    rot.m[3] = 0.0f;

    rot.m[4] = axis[1] * axis[0] * oneMinusCosA + axis[2] * sinA;
    rot.m[5] = cosA + axis[1] * axis[1] * oneMinusCosA;
    rot.m[6] = axis[1] * axis[2] * oneMinusCosA - axis[0] * sinA;
    rot.m[7] = 0.0f;

    rot.m[8] = axis[2] * axis[0] * oneMinusCosA - axis[1] * sinA;
    rot.m[9] = axis[2] * axis[1] * oneMinusCosA + axis[0] * sinA;
    rot.m[10] = cosA + axis[2] * axis[2] * oneMinusCosA;
    rot.m[11] = 0.0f;

    rot.m[12] = 0.0f;
    rot.m[13] = 0.0f;
    rot.m[14] = 0.0f;
    rot.m[15] = 1.0f;

    return rot;
}

static Mat4x4 multiply(const Mat4x4 &a, const Mat4x4 &b) {
    Mat4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i * 4 + j] += a.m[i * 4 + k] * b.m[k * 4 + j];
            }
        }
    }
    return result;
}

Mat4x4 operator*(const Mat4x4 &a, const Mat4x4 &b) {
    return multiply(a, b);
}

Vec3f normalize(const Vec3f &v) {
    float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return Vec3f(v[0] / length, v[1] / length, v[2] / length);
}

namespace Globals {
    float win_width, win_height;
    float near = 0.5f;
    float far = 100.0f;
    float left = -FRUSTUM_WIDTH;
    float right = FRUSTUM_WIDTH;
    float bottom = -FRUSTUM_HEIGHT;
    float top = FRUSTUM_HEIGHT;
    float aspect;

    GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
    TriMesh mesh;
    Vec3f eye = Vec3f(-5.0f, -10.0f, 0.0f);
    Vec3f viewDir = Vec3f(-1.0f, 0.0f, 0.0f);
    Vec3f upDir = Vec3f(0.0f, 1.0f, 0.0f);

    Vec3f u = normalize(viewDir.cross(upDir));
    Vec3f v = normalize(u.cross(upDir));
    Vec3f n = normalize(viewDir * -1.0f);

    Mat4x4 model;
    Mat4x4 view;
    Mat4x4 projection;
}

float cotf(float angle) {
    return 1.0f / tanf(angle);
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float speed = 0.1f;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_R:
                Globals::eye = Vec3f(-5.0f, -10.0f, 0.0f);
                Globals::viewDir = Vec3f(-1.0f, 0.0f, 0.0f);
                break;


            case GLFW_KEY_W:
                Globals::eye += Globals::viewDir * speed;
                break;
            case GLFW_KEY_A:
                Globals::eye += Globals::u * -speed;
                break;
            case GLFW_KEY_S:
                Globals::eye += Globals::viewDir * -speed;
                break;
            case GLFW_KEY_D:
                Globals::eye += Globals::u * speed;
                break;

            case GLFW_KEY_LEFT_BRACKET:
                Globals::eye[1] -= speed;
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                Globals::eye[1] += speed;
                break;

            case GLFW_KEY_UP:
            {
                // 1. Translate p by a vector that causes a to pass through the origin
                Mat4x4 translate;
                translate.m[12] = -Globals::eye[0];
                translate.m[13] = -Globals::eye[1];
                translate.m[14] = -Globals::eye[2];

                // 2. Rotate p by a matrix that causes a to coincide with any axis
                Mat4x4 rotate;
                rotate.m[0] = Globals::u[0];
                rotate.m[4] = Globals::u[1];
                rotate.m[8] = Globals::u[2];
                rotate.m[1] = Globals::v[0];
                rotate.m[5] = Globals::v[1];
                rotate.m[9] = Globals::v[2];
                rotate.m[2] = Globals::n[0];
                rotate.m[6] = Globals::n[1];
                rotate.m[10] = Globals::n[2];

                Globals::viewDir = rotate * (translate * Globals::viewDir);

                // 3. Rotate p about the y axis
                float new_y = Globals::viewDir[1] * cos(speed) - Globals::viewDir[2] * sin(speed);
                float new_z = Globals::viewDir[1] * sin(speed) + Globals::viewDir[2] * cos(speed);
                Globals::viewDir[1] = new_y;
                Globals::viewDir[2] = new_z;

                // 4. Unrotate, Untranslate
                Mat4x4 unrotate;
                unrotate.m[0] = Globals::u[0];
                unrotate.m[1] = Globals::u[1];
                unrotate.m[2] = Globals::u[2];
                unrotate.m[4] = Globals::v[0];
                unrotate.m[5] = Globals::v[1];
                unrotate.m[6] = Globals::v[2];
                unrotate.m[8] = Globals::n[0];
                unrotate.m[9] = Globals::n[1];
                unrotate.m[10] = Globals::n[2];

                Mat4x4 untranslate;
                untranslate.m[12] = Globals::eye[0];
                untranslate.m[13] = Globals::eye[1];
                untranslate.m[14] = Globals::eye[2];

                Globals::viewDir = untranslate * (unrotate * Globals::viewDir);

                Globals::viewDir = rotation(Globals::u, -speed) * Globals::viewDir;
                break;
            }
            case GLFW_KEY_DOWN:
            {
                // 1. Translate p by a vector that causes a to pass through the origin
                Mat4x4 translate;
                translate.m[12] = -Globals::eye[0];
                translate.m[13] = -Globals::eye[1];
                translate.m[14] = -Globals::eye[2];

                // 2. Rotate p by a matrix that causes a to coincide with any axis
                Mat4x4 rotate;
                rotate.m[0] = Globals::u[0];
                rotate.m[4] = Globals::u[1];
                rotate.m[8] = Globals::u[2];
                rotate.m[1] = Globals::v[0];
                rotate.m[5] = Globals::v[1];
                rotate.m[9] = Globals::v[2];
                rotate.m[2] = Globals::n[0];
                rotate.m[6] = Globals::n[1];
                rotate.m[10] = Globals::n[2];

                Globals::viewDir = rotate * (translate * Globals::viewDir);

                // 3. Rotate p about the y axis
                float new_y = Globals::viewDir[1] * cos(-speed) - Globals::viewDir[2] * sin(-speed);
                float new_z = Globals::viewDir[1] * sin(-speed) + Globals::viewDir[2] * cos(-speed);
                Globals::viewDir[1] = new_y;
                Globals::viewDir[2] = new_z;

                // 4. Unrotate, Untranslate
                Mat4x4 unrotate;
                unrotate.m[0] = Globals::u[0];
                unrotate.m[1] = Globals::u[1];
                unrotate.m[2] = Globals::u[2];
                unrotate.m[4] = Globals::v[0];
                unrotate.m[5] = Globals::v[1];
                unrotate.m[6] = Globals::v[2];
                unrotate.m[8] = Globals::n[0];
                unrotate.m[9] = Globals::n[1];
                unrotate.m[10] = Globals::n[2];

                Mat4x4 untranslate;
                untranslate.m[12] = Globals::eye[0];
                untranslate.m[13] = Globals::eye[1];
                untranslate.m[14] = Globals::eye[2];

                Globals::viewDir = untranslate * (unrotate * Globals::viewDir);

                Globals::viewDir = rotation(Globals::u, speed) * Globals::viewDir;
                break;
            }
            case GLFW_KEY_RIGHT:
                Globals::viewDir = rotation_y(-speed) * Globals::viewDir;
                break;
            case GLFW_KEY_LEFT:
                Globals::viewDir = rotation_y(speed) * Globals::viewDir;
                break;
            default:
                break;
        }
    }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Globals::win_width = float(width);
    Globals::win_height = float(height);
    Globals::aspect = Globals::win_width / Globals::win_height;
    glViewport(0, 0, width, height);

    if (Globals::aspect > 1) {
        float newBoundary = (Globals::aspect * (Globals::top - Globals::bottom)) / 2.0f;
        Globals::left = -newBoundary;
        Globals::right = newBoundary;
    } else {
        float newBoundary = ((Globals::right - Globals::left) / Globals::aspect) / 2.0f;
        Globals::bottom = -newBoundary;
        Globals::top = newBoundary;
    }


    Globals::projection.m[0] = 2 * Globals::near / (Globals::right - Globals::left);
    Globals::projection.m[5] = 2 * Globals::near / (Globals::top - Globals::bottom);
    Globals::projection.m[8] = (Globals::right + Globals::left) / (Globals::right - Globals::left);
    Globals::projection.m[9] = (Globals::top + Globals::bottom) / (Globals::top - Globals::bottom);
    Globals::projection.m[10] = -(Globals::far + Globals::near) / (Globals::far - Globals::near);
    Globals::projection.m[11] = -1;
    Globals::projection.m[14] = -2 * Globals::far * Globals::near / (Globals::far - Globals::near);
    Globals::projection.m[15] = 0;
}


void init_scene() {
    using namespace Globals;

    glGenBuffers(1, verts_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, normals_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, faces_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size() * sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &tris_vao);
    glBindVertexArray(tris_vao);

    int vert_dim = 3;

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
    glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
    glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
    glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), 0);

    glBindVertexArray(0);
}

int main() {
    std::stringstream obj_file;
    obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
    if (!Globals::mesh.load_obj(obj_file.str())) { return 0; }
    Globals::mesh.print_details();

    GLFWwindow* window;
    glfwSetErrorCallback(&error_callback);
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    Globals::win_width = WIN_WIDTH;
    Globals::win_height = WIN_HEIGHT;
    window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2b", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(window, &key_callback);
    glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // make sure the openGL code can be found; folks using Windows need this
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to gladLoadGLLoader" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    mcl::Shader shader;
    std::stringstream ss;
    ss << MY_SRC_DIR << "shader.";
    shader.init_from_files(ss.str() + "vert", ss.str() + "frag");

    init_scene();
    framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height));

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.f, 1.f, 1.f, 1.f);

    shader.enable();

    glBindVertexArray(Globals::tris_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Globals::n = normalize(Globals::viewDir * -1.0f);
        Globals::u = normalize(Globals::upDir.cross(Globals::n));
        Globals::v = normalize(Globals::n.cross(Globals::u));
        float dx = -(Globals::eye.dot(Globals::u));
        float dy = -(Globals::eye.dot(Globals::v));
        float dz = -(Globals::eye.dot(Globals::n));
        Globals::view.m[0] = Globals::u[0];
        Globals::view.m[1] = Globals::v[0];
        Globals::view.m[2] = Globals::n[0];
        Globals::view.m[4] = Globals::u[1];
        Globals::view.m[5] = Globals::v[1];
        Globals::view.m[6] = Globals::n[1];
        Globals::view.m[8] = Globals::u[2];
        Globals::view.m[9] = Globals::v[2];
        Globals::view.m[10] = Globals::n[2];
        Globals::view.m[12] = dx;
        Globals::view.m[13] = dy;
        Globals::view.m[14] = dz;

//        printf("M = [%f %f %f %f\n     %f %f %f %f\n     %f %f %f %f\n     %f %f %f %f]\n", Globals::projection.m[0], Globals::projection.m[4], Globals::projection.m[8], Globals::projection.m[12], Globals::projection.m[1], Globals::projection.m[5], Globals::projection.m[9], Globals::projection.m[13], Globals::projection.m[2], Globals::projection.m[6], Globals::projection.m[10], Globals::projection.m[14], Globals::projection.m[3], Globals::projection.m[7], Globals::projection.m[11], Globals::projection.m[15]);
//        printf("M = [%f %f %f %f\n     %f %f %f %f\n     %f %f %f %f\n     %f %f %f %f]\n", Globals::view.m[0], Globals::view.m[4], Globals::view.m[8], Globals::view.m[12], Globals::view.m[1], Globals::view.m[5], Globals::view.m[9], Globals::view.m[13], Globals::view.m[2], Globals::view.m[6], Globals::view.m[10], Globals::view.m[14], Globals::view.m[3], Globals::view.m[7], Globals::view.m[11], Globals::view.m[15]);

        glUniformMatrix4fv((GLint) shader.uniform("model"), 1, GL_FALSE, Globals::model.m);
        glUniformMatrix4fv((GLint) shader.uniform("view"), 1, GL_FALSE, Globals::view.m);
        glUniformMatrix4fv((GLint) shader.uniform("projection"), 1, GL_FALSE, Globals::projection.m);

        glDrawElements(GL_TRIANGLES, (GLint) Globals::mesh.faces.size() * 3, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.disable();

    return EXIT_SUCCESS;
}
