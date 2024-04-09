#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "trimesh.hpp"
#include "shader.hpp"

#define WIN_WIDTH 500
#define WIN_HEIGHT 500

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
};

static inline Vec3f operator*(const Mat4x4 &m, const Vec3f &v) {
    Vec3f r(m.m[0] * v[0] + m.m[4] * v[1] + m.m[8] * v[2],
            m.m[1] * v[0] + m.m[5] * v[1] + m.m[9] * v[2],
            m.m[2] * v[0] + m.m[6] * v[1] + m.m[10] * v[2]);
    return r;
}

static Mat4x4 rotation_y(float angle) {
    Mat4x4 rot;
    float cosA = cos(angle);
    float sinA = sin(angle);
    rot.m[0] = cosA;
    rot.m[2] = -sinA;
    rot.m[8] = sinA;
    rot.m[10] = cosA;
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


namespace Globals {
    float win_width, win_height;
    float aspect;
    GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
    TriMesh mesh;

    Mat4x4 model;
    Mat4x4 view;
    Mat4x4 projection;
}

static Mat4x4 perspective(float aspect) {
    Mat4x4 proj;
    float f = 1.0f / tanf(45.0 * 0.5f * (M_PI / 180.0f));
    proj.m[0] = f / aspect;
    proj.m[5] = f;
    proj.m[10] = (100.0 + 0.1) / (0.1 - 100.0);
    proj.m[11] = -1.0f;
    proj.m[14] = (2.0f * 100.0 * 0.1) / (0.1 - 100.0);
    proj.m[15] = 0.0f;
    return proj;
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

Vec3f normalize(const Vec3f &v) {
    float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return Vec3f(v[0] / length, v[1] / length, v[2] / length);
}


void resetView() {
    Globals::view.make_identity();
    Vec3f cameraForward(0.0f, 0.0f, -1.0f);
    Vec3f cameraRight(1.0f, 0.0f, 0.0f);
    Vec3f cameraUp(0.0f, 1.0f, 0.0f);

    Mat4x4 rotationMatrix;
    rotationMatrix.m[0] = cameraRight[0];
    rotationMatrix.m[1] = cameraRight[1];
    rotationMatrix.m[2] = cameraRight[2];
    rotationMatrix.m[4] = cameraUp[0];
    rotationMatrix.m[5] = cameraUp[1];
    rotationMatrix.m[6] = cameraUp[2];
    rotationMatrix.m[8] = -cameraForward[0];
    rotationMatrix.m[9] = -cameraForward[1];
    rotationMatrix.m[10] = -cameraForward[2];

    Globals::view = rotationMatrix;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float move_speed = 0.1f;
        float rotate_speed = 0.1f;
        float translation_speed = 0.1f;
        float rotation_speed = 0.1f;
        float angle;
        switch (key) {
            case GLFW_KEY_LEFT_BRACKET:
                Globals::view.m[13] -= translation_speed;
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                Globals::view.m[13] += translation_speed;
                break;
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_W:
                Globals::view.m[14] += move_speed;
                break;
            case GLFW_KEY_S:
                Globals::view.m[14] -= move_speed;
                break;
            case GLFW_KEY_A:
                Globals::view = Globals::view * rotation_y(-rotate_speed);
                break;
            case GLFW_KEY_D:
                Globals::view = Globals::view * rotation_y(rotate_speed);
                break;
            case GLFW_KEY_R:
                resetView();
                break;
            case GLFW_KEY_UP:
                angle = rotation_speed;
                break;
            case GLFW_KEY_DOWN:
                angle = -rotation_speed;
                break;
            default:
                break;
        }

        // TODO: fix this axis?
        Vec3f view_direction = Vec3f(-Globals::view.m[8], -Globals::view.m[9], -Globals::view.m[10]);
        Vec3f up(0.0f, 1.0f, 0.0f);
        Vec3f rotation_axis = normalize(view_direction.cross(up));
        Mat4x4 rotation_matrix;
        float cos_theta = cos(angle);
        float sin_theta = sin(angle);
        float one_minus_cos_theta = 1.0f - cos_theta;
        rotation_matrix.m[0] = cos_theta + rotation_axis[0] * rotation_axis[0] * one_minus_cos_theta;
        rotation_matrix.m[1] = rotation_axis[0] * rotation_axis[1] * one_minus_cos_theta - rotation_axis[2] * sin_theta;
        rotation_matrix.m[2] = rotation_axis[0] * rotation_axis[2] * one_minus_cos_theta + rotation_axis[1] * sin_theta;
        rotation_matrix.m[4] = rotation_axis[1] * rotation_axis[0] * one_minus_cos_theta + rotation_axis[2] * sin_theta;
        rotation_matrix.m[5] = cos_theta + rotation_axis[1] * rotation_axis[1] * one_minus_cos_theta;
        rotation_matrix.m[6] = rotation_axis[1] * rotation_axis[2] * one_minus_cos_theta - rotation_axis[0] * sin_theta;
        rotation_matrix.m[8] = rotation_axis[2] * rotation_axis[0] * one_minus_cos_theta - rotation_axis[1] * sin_theta;
        rotation_matrix.m[9] = rotation_axis[2] * rotation_axis[1] * one_minus_cos_theta + rotation_axis[0] * sin_theta;
        rotation_matrix.m[10] = cos_theta + rotation_axis[2] * rotation_axis[2] * one_minus_cos_theta;
        Globals::view = rotation_matrix * Globals::view;
    }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Globals::win_width = static_cast<float>(width);
    Globals::win_height = static_cast<float>(height);
    Globals::aspect = Globals::win_width / Globals::win_height;
    glViewport(0, 0, width, height);
    Globals::projection = perspective(Globals::aspect);
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
