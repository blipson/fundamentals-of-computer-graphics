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
} Limb;

int numLimbs = 3;
Limb* limbs = (Limb*) malloc(numLimbs * sizeof(Limb));

enum Operation {
    BASE,
    ROTATE,
    ROTATE_LIMB,
    TRANSLATE,
    SCALE,
    RESET,
};

GLint window_width = 500;
GLint window_height = 500;
GLfloat pi = 4.0f*atanf(1.0);

GLint numberOfVertices = 90000;

typedef struct {
    GLdouble mouseX;
    GLdouble mouseY;
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
    .operation = RESET,
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

static void error_callback(int error, const char* description){
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    switch (key) {
        case GLFW_KEY_LEFT:
            state.operation = SCALE;
            if (state.scaleFactorX > 0.01) {
                state.scaleFactorX *= 0.9;
            }
            break;
        case GLFW_KEY_RIGHT:
            state.operation = SCALE;
            state.scaleFactorX *= 1.1;
            break;
        case GLFW_KEY_UP:
            state.operation = SCALE;
            state.scaleFactorY *= 1.1;
            break;
        case GLFW_KEY_DOWN:
            state.operation = SCALE;
            if (state.scaleFactorY > 0.01) {
                state.scaleFactorY *= 0.9;
            }
            break;
        case GLFW_KEY_R:
            state.operation = RESET;
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

FloatType2D computeCentroid(FloatType2D vertices[], int numVertices) {
    FloatType2D result;
    result.x = 0.0f;
    result.y = 0.0f;
    for (int i = 0; i < numVertices; ++i) {
        result.x += vertices[i].x;
        result.y += vertices[i].y;
    }
    result.x /= (float) numVertices;
    result.y /= (float) numVertices;
    return result;
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
        if (state.rotationAngle > 2 * pi) {
            state.rotationAngle -= 2 * pi;
        }
        else if (state.rotationAngle < - 2 * pi) {
            state.rotationAngle += 2 * pi;
        }
    } else if (state.operation == ROTATE_LIMB) {
        state.limbRotationAngle += 2.0f * (float) dx / (float) window_width * pi;
        if (state.limbRotationAngle > 2 * pi) {
            state.limbRotationAngle -= 2 * pi;
        } else if (state.limbRotationAngle < - 2 * pi) {
            state.limbRotationAngle += 2 * pi;
        }
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
    for (int i = 0; i < numLimbs; i++) {
        limbs[i] = (Limb) {
                .vertices = (FloatType2D*) malloc(3 * sizeof(FloatType2D)),
                .numVertices = 0,
                .colors= (ColorType3D*) malloc(3 * sizeof(ColorType3D)),
        };
    }
    std::string line;
    numVertices = 0;
    int currentLimb = 0;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        char type;
        iss >> type;
        float z;

        if (limbs[currentLimb].numVertices < 3) {
            if (type == 'v') {
                float x, y, r, g, b;
                iss >> x >> y >> z >> r >> g >> b;
                limbs[currentLimb].vertices[limbs[currentLimb].numVertices].x = x;
                limbs[currentLimb].vertices[limbs[currentLimb].numVertices].y = y;
                limbs[currentLimb].colors[limbs[currentLimb].numVertices].r = r;
                limbs[currentLimb].colors[limbs[currentLimb].numVertices].g = g;
                limbs[currentLimb].colors[limbs[currentLimb].numVertices].b = b;
                limbs[currentLimb].numVertices++;

                vertices[numVertices].x = x;
                vertices[numVertices].y = y;
                colors[numVertices].r = r;
                colors[numVertices].g = g;
                colors[numVertices].b = b;
                numVertices++;
                if (numVertices % 3 == 0) {
                    currentLimb++;
                }
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
}

void multiplyMatrices(GLfloat result[16], const GLfloat matrix1[16], const GLfloat matrix2[16]) {
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            result[col * 4 + row] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[col * 4 + row] += matrix1[k * 4 + row] * matrix2[col * 4 + k];
            }
        }
    }
}


int main() {
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

    init(&transformationMatrixLocation);

    // Q: what's the difference between triangles and triangle strips?


    GLfloat transformationMatrix[16];
    GLfloat scalingMatrix[16];
    for (int i = 0; i < 16; i++) {
        scalingMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    GLfloat scalingApplied[16];
    GLfloat rotationMatrix[16];
    for (int i = 0; i < 16; i++) {
        rotationMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    GLfloat rotationApplied[16];
    GLfloat translationApplied[16];


    while (!glfwWindowShouldClose(window)) {
		glClear( GL_COLOR_BUFFER_BIT );

        if (DEBUG_ON){
            printf("M = [%f %f %f %f\n     %f %f %f %f\n     %f %f %f %f\n     %f %f %f %f]\n", transformationMatrix[0], transformationMatrix[4], transformationMatrix[8], transformationMatrix[12], transformationMatrix[1], transformationMatrix[5], transformationMatrix[9], transformationMatrix[13], transformationMatrix[2], transformationMatrix[6], transformationMatrix[10], transformationMatrix[14], transformationMatrix[3], transformationMatrix[7], transformationMatrix[11], transformationMatrix[15]);
        }

        // --------------- LIMB --------------- //
        for (int i = 0; i < numLimbs; i++) {
            GLfloat limbTransformationMatrix[16];
            GLfloat limbRotationMatrix[16];
            for (int j = 0; j < 16; j++) {
                limbRotationMatrix[j] = (j % 5 == 0) ? 1.0f : 0.0f;
            }
            GLfloat limbRotationApplied[16];
            GLfloat limbRotationOnPivotApplied[16];
            GLfloat limbTranslationToOriginApplied[16];
            GLfloat limbTranslationBackApplied[16];
            GLfloat limbGlobalScalingMatrix[16];
            for (int j = 0; j < 16; j++) {
                limbGlobalScalingMatrix[j] = (j % 5 == 0) ? 1.0f : 0.0f;
            }
            GLfloat limbGlobalScalingApplied[16];
            GLfloat limbGlobalRotationMatrix[16];
            for (int j = 0; j < 16; j++) {
                limbGlobalRotationMatrix[j] = (j % 5 == 0) ? 1.0f : 0.0f;
            }
            GLfloat limbGlobalRotationApplied[16];
            GLfloat limbGlobalTranslationApplied[16];

            for (int j = 0; j < 16; j++) {
                limbTransformationMatrix[j] = (j % 5 == 0) ? 1.0f : 0.0f;
            }
            state.centroid = computeCentroid(limbs[i].vertices, limbs[i].numVertices);

            FloatType2D scaledCentroid = (FloatType2D) {
                    .x = state.centroid.x * state.scaleFactorX,
                    .y = state.centroid.y * state.scaleFactorY
            };

            // translate the limb to 0,0
            for (int j = 0; j < 16; j++) {
                if (j == 12) {
                    limbTranslationToOriginApplied[j] = limbTransformationMatrix[j] - scaledCentroid.x;
                } else if (j == 13) {
                    limbTranslationToOriginApplied[j] = limbTransformationMatrix[j] - scaledCentroid.y;
                } else {
                    limbTranslationToOriginApplied[j] = limbTransformationMatrix[j];
                }
            }

            // apply limb rotation
            GLfloat cosLimbTheta = cos(state.limbRotationAngle);
            GLfloat sinLimbTheta = sin(state.limbRotationAngle);
            limbRotationMatrix[0] = cosLimbTheta;
            limbRotationMatrix[1] = -sinLimbTheta;
            limbRotationMatrix[4] = sinLimbTheta;
            limbRotationMatrix[5] = cosLimbTheta;
            multiplyMatrices(limbRotationApplied, limbTransformationMatrix, limbRotationMatrix);

            // Q: Why is this in the opposite order?
            multiplyMatrices(limbRotationOnPivotApplied, limbRotationApplied, limbTranslationToOriginApplied);

            // translate the limb back
            for (int j = 0; j < 16; j++) {
                if (j == 12) {
                    limbTranslationBackApplied[j] = limbRotationOnPivotApplied[j] + scaledCentroid.x;
                } else if (j == 13) {
                    limbTranslationBackApplied[j] = limbRotationOnPivotApplied[j] + scaledCentroid.y;
                } else {
                    limbTranslationBackApplied[j] = limbRotationOnPivotApplied[j];
                }
            }
            // apply global scaling
            limbGlobalScalingMatrix[0] = state.scaleFactorX;
            limbGlobalScalingMatrix[5] = state.scaleFactorY;
            multiplyMatrices(limbGlobalScalingApplied, limbTranslationBackApplied, limbGlobalScalingMatrix);

            // apply global rotation
            GLfloat globalCosTheta = cos(state.rotationAngle);
            GLfloat globalSinTheta = sin(state.rotationAngle);
            limbGlobalRotationMatrix[0] = globalCosTheta;
            limbGlobalRotationMatrix[1] = -globalSinTheta;
            limbGlobalRotationMatrix[4] = globalSinTheta;
            limbGlobalRotationMatrix[5] = globalCosTheta;
            multiplyMatrices(limbGlobalRotationApplied, limbGlobalRotationMatrix, limbGlobalScalingApplied);

            // apply global translation
            for (int j = 0; j < 16; j++) {
                if (j == 12) {
                    limbGlobalTranslationApplied[j] = limbGlobalRotationApplied[j] + state.translateX;
                } else if (j == 13) {
                    limbGlobalTranslationApplied[j] = limbGlobalRotationApplied[j] + state.translateY;
                } else {
                    limbGlobalTranslationApplied[j] = limbGlobalRotationApplied[j];
                }
            }

            glUniformMatrix4fv(transformationMatrixLocation, 1, GL_FALSE, limbGlobalTranslationApplied);
            glDrawArrays(GL_TRIANGLES, i * 3, limbs[i].numVertices);
        }

        // --------------- REST OF MODEL --------------- //
        // set the transformation matrix to the unit matrix
        for (int i = 0; i < 16; i++) {
            transformationMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        }

        // apply global scaling
        scalingMatrix[0] = state.scaleFactorX;
        scalingMatrix[5] = state.scaleFactorY;
        multiplyMatrices(scalingApplied, transformationMatrix, scalingMatrix);

        // apply global rotation
        GLfloat cosTheta = cos(state.rotationAngle);
        GLfloat sinTheta = sin(state.rotationAngle);
        rotationMatrix[0] = cosTheta;
        rotationMatrix[1] = -sinTheta;
        rotationMatrix[4] = sinTheta;
        rotationMatrix[5] = cosTheta;
        multiplyMatrices(rotationApplied, scalingApplied, rotationMatrix);

        // apply global translation
        for (int i = 0; i < 16; i++) {
            if (i == 12) {
                translationApplied[i] = rotationApplied[i] + state.translateX;
            } else if (i == 13) {
                translationApplied[i] = rotationApplied[i] + state.translateY;
            } else {
                translationApplied[i] = rotationApplied[i];
            }
        }

        glUniformMatrix4fv(transformationMatrixLocation, 1, GL_FALSE, translationApplied );
//        glDrawArrays(GL_TRIANGLES, 3, numberOfVertices );

		glFlush();
        glfwSwapBuffers(window);
        glfwWaitEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
