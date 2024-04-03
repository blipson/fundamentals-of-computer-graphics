#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cfloat>

#define DEBUG_ON 0

#include "ShaderStuff.hpp"

typedef struct {
    GLfloat x, y;
} FloatType2D;

typedef struct {
    GLfloat r, g, b;
} ColorType3D;

typedef struct {
    FloatType2D* vertices;
    int numVertices;
    ColorType3D* colors;
    GLfloat rotationAngle;
} Limb;

int numLimbs = 0;
Limb* limbs = (Limb*) malloc(1 * sizeof(Limb));

enum Operation {
    BASE,
    ROTATE,
    ROTATE_LIMB,
    TRANSLATE
};

GLint window_width = 500;
GLint window_height = 500;
GLfloat pi = 4.0f*atanf(1.0);

GLFWcursor *hand_cursor, *arrow_cursor;

GLint numberOfVertices = 90000;

typedef struct {
    GLdouble mouseX;
    GLdouble mouseY;
//    GLFWcursor* handCursor;
//    GLFWcursor* arrowCursor;
    Operation operation;
    GLdouble previousMouseX;
    GLdouble previousMouseY;
    GLfloat rotationAngle;
    GLfloat limbRotationAngle;
    GLfloat scaleFactorX;
    GLfloat scaleFactorY;
    GLfloat translateX;
    GLfloat translateY;
    FloatType2D centroid;
} State;

State state = (State) {
    .mouseX = 0,
    .mouseY = 0,
    .operation = BASE,
    .previousMouseX = 0,
    .previousMouseY = 0,
    .rotationAngle = 0,
    .scaleFactorX = 1,
    .scaleFactorY = 1,
    .translateX = 0,
    .translateY = 0,
    .centroid = (FloatType2D) { .x = 0, .y = 0 },
};

void resetMatrix() {
    state.previousMouseX = 0.0;
    state.previousMouseY = 0.0;
    state.rotationAngle = 0.0;
    state.limbRotationAngle = 0.0;
    state.scaleFactorX = 1.0;
    state.scaleFactorY = 1.0;
    state.translateX = 0.0;
    state.translateY = 0.0;
}

// TODO: use this???
void computeCentroid(FloatType2D vertices[], int numVertices, FloatType2D& centroid) {
    centroid.x = 0.0f;
    centroid.y = 0.0f;
    for (int i = 0; i < numVertices; ++i) {
        FloatType2D scaledVertex;
        scaledVertex.x = vertices[i].x * state.scaleFactorX;
        scaledVertex.y = vertices[i].y * state.scaleFactorY;
        centroid.x += scaledVertex.x;
        centroid.y += scaledVertex.y;
    }
    centroid.x /= numVertices;
    centroid.y /= numVertices;
}

void updateMatrix(GLfloat limbTransformationMatrix[16]) {
    for (int i = 0; i < 16; i++) {
        limbTransformationMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }

    GLfloat cosTheta = cos(state.limbRotationAngle + state.rotationAngle);
    GLfloat sinTheta = sin(state.limbRotationAngle + state.rotationAngle);

    limbTransformationMatrix[0] = cosTheta * state.scaleFactorX;
    limbTransformationMatrix[1] = -sinTheta * state.scaleFactorY;
    limbTransformationMatrix[4] = sinTheta * state.scaleFactorX;
    limbTransformationMatrix[5] = cosTheta * state.scaleFactorY;
    limbTransformationMatrix[8] = cosTheta * state.scaleFactorX;

    limbTransformationMatrix[12] = state.translateX + state.centroid.x - state.centroid.y * sinTheta - state.centroid.x * cosTheta;
    limbTransformationMatrix[13] = state.translateY + state.centroid.y - state.centroid.x * sinTheta - state.centroid.y * cosTheta;
}


static void error_callback(int error, const char* description){
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    switch (key) {
        case GLFW_KEY_LEFT:
            if (state.scaleFactorX > 0.01) {
                state.scaleFactorX *= 0.9;
            }
            break;
        case GLFW_KEY_RIGHT:
            state.scaleFactorX *= 1.1;
            break;
        case GLFW_KEY_UP:
            state.scaleFactorY *= 1.1;
            break;
        case GLFW_KEY_DOWN:
            if (state.scaleFactorY > 0.01) {
                state.scaleFactorY *= 0.9;
            }
            break;
        case GLFW_KEY_R:
            resetMatrix();
            break;
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        default:
            break;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    glfwGetCursorPos(window, &state.mouseX, &state.mouseY);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods != GLFW_MOD_CONTROL && mods != GLFW_MOD_SHIFT) {
        state.operation = ROTATE;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        state.operation = BASE;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_CONTROL) {
        state.operation = TRANSLATE;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_SHIFT) {
        state.operation = ROTATE_LIMB;
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos) {
    double dx = xPos - state.previousMouseX;
    double dy = yPos - state.previousMouseY;
    state.previousMouseX = xPos;
    state.previousMouseY = yPos;
    if (xPos < 0 || xPos > window_width || yPos < 0 || yPos > window_height) {
        return;
    }
    if (state.operation == ROTATE) {
        state.rotationAngle += 2.0f * (float) dx / (float) window_width * pi;
        if (state.rotationAngle > 2 * pi)
            state.rotationAngle -= 2 * pi;
        else if (state.rotationAngle < - 2 * pi)
            state.rotationAngle += 2 * pi;
    } else if (state.operation == ROTATE_LIMB) {
        state.limbRotationAngle += 2.0f * (float) dx / (float) window_width * pi;
        if (state.limbRotationAngle > 2 * pi)
            state.limbRotationAngle -= 2 * pi;
        else if (state.limbRotationAngle < - 2 * pi)
            state.limbRotationAngle += 2 * pi;
    } else if (state.operation == TRANSLATE) {
        if ((dx > 0 && state.translateX < 0.99) || (dx < 0 && state.translateX > -0.99)) {
            state.translateX += (float) dx / ((float) window_width/ 2);
        }
        if ((dy < 0 && state.translateY < 0.99) || (dy > 0 && state.translateY > -0.99)) {
            state.translateY += -((float) dy) / ((float) window_height / 2);
        }
    }
}

void readVerticesFromFile(const std::string& filename, FloatType2D vertices[], ColorType3D colors[], int& numVertices) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    limbs[0] = (Limb) {
            .vertices = (FloatType2D *) malloc(3 * sizeof(FloatType2D)),
            .numVertices = 0,
            .colors= (ColorType3D *) malloc(3 * sizeof(ColorType3D)),
            .rotationAngle = 0.0f,
    };
    std::string line;
    numVertices = 0;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        char type;
        iss >> type;
        float z;

        if (limbs[0].numVertices < 3) {
            if (type == 'v') {
                float x, y, r, g, b;
                iss >> x >> y >> z >> r >> g >> b;
                limbs[0].vertices[limbs[0].numVertices].x = x;
                limbs[0].vertices[limbs[0].numVertices].y = y;
                limbs[0].colors[limbs[0].numVertices].r = r;
                limbs[0].colors[limbs[0].numVertices].g = g;
                limbs[0].colors[limbs[0].numVertices].b = b;
                limbs[0].numVertices++;

                vertices[numVertices].x = x;
                vertices[numVertices].y = y;
                colors[numVertices].r = r;
                colors[numVertices].g = g;
                colors[numVertices].b = b;
                numVertices++;
            }
        } else {
            if (type == 'v') {
                iss >> vertices[numVertices].x >> vertices[numVertices].y >> z >> colors[numVertices].r >> colors[numVertices].g >> colors[numVertices].b;
                numVertices++;
            }
        }
    }

    file.close();
}

void scaleAndTranslate(FloatType2D vertices[], int numVertices) {
    FloatType2D minCoords = {FLT_MAX, FLT_MAX};
    FloatType2D maxCoords = {-FLT_MAX, -FLT_MAX};
    for (int i = 0; i < numVertices; ++i) {
        minCoords.x = std::min(minCoords.x, vertices[i].x);
        minCoords.y = std::min(minCoords.y, vertices[i].y);
        maxCoords.x = std::max(maxCoords.x, vertices[i].x);
        maxCoords.y = std::max(maxCoords.y, vertices[i].y);
    }

    FloatType2D scaleFactors = {2.0f / (maxCoords.x - minCoords.x), 2.0f / (maxCoords.y - minCoords.y)};
    FloatType2D translation = {-(maxCoords.x + minCoords.x) / 2.0f, -(maxCoords.y + minCoords.y) / 2.0f};

    for (int i = 0; i < numVertices; ++i) {
        vertices[i].x = (vertices[i].x + translation.x) * scaleFactors.x;
        vertices[i].y = (vertices[i].y + translation.y) * scaleFactors.y;
    }
}

void init(GLint* transformationMatrixLocation)
{
    ColorType3D colors[numberOfVertices];
    FloatType2D vertices[numberOfVertices];
    int numVertices = 0;
    GLuint vao[1], buffer, program, location1, location2;

    readVerticesFromFile("/Users/Z003YW4/github.com/fundamentals-of-computer-graphics/assignment2a/src/model.txt", vertices, colors, numVertices);
    // needed for .obj files
    // scaleAndTranslate(vertices, numVertices);

    glGenVertexArrays( 1, vao );
    glBindVertexArray( vao[0] );
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(colors), vertices, GL_STATIC_DRAW );
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

    std::stringstream vshader, fshader;
    vshader << SRC_DIR << "/vshader2a.glsl";
    fshader << SRC_DIR << "/fshader2a.glsl";

    program = InitShader( vshader.str().c_str(), fshader.str().c_str() );

    location1 = glGetAttribLocation( program, "vertex_position" );
    glEnableVertexAttribArray( location1 );
    glVertexAttribPointer( location1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    location2 = glGetAttribLocation( program, "vertex_color" );
    glEnableVertexAttribArray( location2 );
    glVertexAttribPointer( location2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)) );
    *transformationMatrixLocation = glGetUniformLocation(program, "transformationMatrix");

    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
}

int main() {
    GLfloat transformationMatrix[16];
    GLint transformationMatrixLocation;
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	window = glfwCreateWindow(window_width, window_height, "HW2a", NULL, NULL);

    if (!window) {
        printf("GLFW failed to create window; terminating\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

    // Load all OpenGL functions (needed if using Windows)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("gladLoadGLLoader failed; terminating\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(1);  // tells the system to wait for the rendered frame to finish updating before swapping buffers; can help to avoid tearing
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

	// TODO: fix this
    init(&transformationMatrixLocation);
    GLfloat limbTransformationMatrix[16];

    for (int i = 0; i < 16; i++) {
        limbTransformationMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    glDrawArrays(GL_TRIANGLES, 3, numberOfVertices );
    glDrawArrays(GL_TRIANGLES, 0, limbs[0].numVertices);
    while (!glfwWindowShouldClose(window)) {
		glClear( GL_COLOR_BUFFER_BIT );

        if (DEBUG_ON){
            printf("M = [%f %f %f %f\n     %f %f %f %f\n     %f %f %f %f\n     %f %f %f %f]\n", transformationMatrix[0], transformationMatrix[4], transformationMatrix[8], transformationMatrix[12], transformationMatrix[1], transformationMatrix[5], transformationMatrix[9], transformationMatrix[13], transformationMatrix[2], transformationMatrix[6], transformationMatrix[10], transformationMatrix[14], transformationMatrix[3], transformationMatrix[7], transformationMatrix[11], transformationMatrix[15]);
        }

        if (state.operation == ROTATE_LIMB) {
            state.centroid.x = 0;
            state.centroid.y = 0.58333f * state.scaleFactorY;
            updateMatrix(limbTransformationMatrix);
        } else {
            state.centroid.x = 0;
            state.centroid.y = 0;
            updateMatrix(transformationMatrix);
            updateMatrix(limbTransformationMatrix);
        }

        glUniformMatrix4fv(transformationMatrixLocation, 1, GL_FALSE, limbTransformationMatrix);
        glDrawArrays(GL_TRIANGLES, 0, limbs[0].numVertices);
        glUniformMatrix4fv(transformationMatrixLocation, 1, GL_FALSE, transformationMatrix );
        glDrawArrays(GL_TRIANGLES, 3, numberOfVertices );

		glFlush();
        glfwSwapBuffers(window);
        glfwWaitEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
