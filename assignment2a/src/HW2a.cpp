// Skeleton code for hw2a
// Based on example code from: Interactive Computer Graphics: A Top-Down Approach with Shader-Based OpenGL (6th Edition), by Ed Angel


#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream> // For file input
#include <string> // For string manipulation
#include <cfloat>


#define DEBUG_ON 0  // repetitive of the debug flag in the shader loading code, included here for clarity only

// This file contains the code that reads the shaders from their files and compiles them
#include "ShaderStuff.hpp"

//----------------------------------------------------------------------------

// initialize some basic structure types
typedef struct {
    GLfloat x, y;
} FloatType2D;

typedef struct {
    GLfloat r, g, b;
} ColorType3D;

enum State {
    BASE,
    ROTATE,
    ROTATE_RIGHT,
    TRANSLATE
};

GLfloat M[16]; // general transformation matrix

// define some assorted global variables, to make life easier
GLint m_location;
GLdouble mouse_x, mouse_y;
GLint window_width = 500;
GLint window_height = 500;
GLdouble pi = 4.0*atan(1.0);

GLFWcursor *hand_cursor, *arrow_cursor; // some different cursors

GLint NVERTICES = 90000; // part of the hard-coded model

State state = BASE;
double previous_xpos = 0.0;
double previous_ypos = 0.0;

double rotation_angle = 0.0;

double scale_factor_x = 1.0;
double scale_factor_y = 1.0;
double translate_x = 0.0;
double translate_y = 0.0;

void resetMatrix() {
    previous_xpos = 0.0;
    previous_ypos = 0.0;

    rotation_angle = 0.0;

    scale_factor_x = 1.0;
    scale_factor_y = 1.0;
    translate_x = 0.0;
    translate_y = 0.0;
}

void updateMatrix() {
    // Reset to identity matrix
    for (int i = 0; i < 16; i++) {
        M[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }

    double cosTheta = cos(rotation_angle);
    double sinTheta = sin(rotation_angle);

    // Apply transformations
    M[0] = cos(rotation_angle) * scale_factor_x;
    M[1] = -sin(rotation_angle) * scale_factor_y;
    M[4] = sin(rotation_angle) * scale_factor_x;
    M[5] = cos(rotation_angle) * scale_factor_y;
    M[8] = cos(rotation_angle) * scale_factor_x;

    M[12] = translate_x;
    M[13] = translate_y;
}


//----------------------------------------------------------------------------
// function that is called whenever an error occurs
static void
error_callback(int error, const char* description){
    fputs(description, stderr);  // write the error description to stderr
}

//----------------------------------------------------------------------------
// function that is called whenever a keyboard event occurs; defines how keyboard input will be handled
static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) // checks to see if the escape key was pressed
    glfwSetWindowShouldClose(window, GL_TRUE);  // closes the window

    switch (key) {
        case GLFW_KEY_LEFT:
            if (scale_factor_x > 0.01) {
                scale_factor_x *= 0.9;
            }
            break;
        case GLFW_KEY_RIGHT:
            scale_factor_x *= 1.1;
            break;
        case GLFW_KEY_UP:
            scale_factor_y *= 1.1;
            break;
        case GLFW_KEY_DOWN:
            if (scale_factor_y > 0.01) {
                scale_factor_y *= 0.9;
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

//----------------------------------------------------------------------------
// function that is called whenever a mouse or trackpad button press event occurs
static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods != GLFW_MOD_CONTROL) {
        state = ROTATE;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        state = BASE;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_CONTROL) {
        state = TRANSLATE;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_ALT) {
        state = ROTATE_RIGHT;
    }

    // Check which mouse button triggered the event, e.g. GLFW_MOUSE_BUTTON_LEFT, etc.
    // and what the button action was, e.g. GLFW_PRESS, GLFW_RELEASE, etc.
    // (Note that ordinary trackpad click = mouse left button)
    // Also check if any modifier keys were active at the time of the button press, e.g. GLFW_MOD_ALT, etc.
    // Take the appropriate action, which could (optionally) also include changing the cursor's appearance

}

//----------------------------------------------------------------------------
// function that is called whenever a cursor motion event occurs
static void
cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    double dx = xpos - previous_xpos;
    double dy = ypos - previous_ypos;
    previous_xpos = xpos;
    previous_ypos = ypos;
    if (xpos < 0 || xpos > window_width || ypos < 0 || ypos > window_height) {
        return;
    }
    if (state == ROTATE) {
        rotation_angle += 2 * dx / window_width * pi;
        if (rotation_angle > 2 * pi)
            rotation_angle -= 2 * pi;
        else if (rotation_angle < - 2 * pi)
            rotation_angle += 2 * pi;
    } else if (state == TRANSLATE) {
        if ((dx > 0 && translate_x < 0.99) || (dx < 0 && translate_x > -0.99)) {
            translate_x += dx / (window_width / 2);
        }
        if ((dy < 0 && translate_y < 0.99) || (dy > 0 && translate_y > -0.99)) {
            translate_y += -dy / (window_height / 2);
        }
    } else if (state == ROTATE_RIGHT) {

    }
}

// Function to read 3D object or scene description from file
void readFile(const std::string& filename, FloatType2D vertices[], ColorType3D colors[], int& numVertices) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    numVertices = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        char type;
        iss >> type;
        float z;

        if (type == 'v') {
            iss >> vertices[numVertices].x >> vertices[numVertices].y >> z >> colors[numVertices].r >> colors[numVertices].g >> colors[numVertices].b;
//            vertices[numVertices].x /= 300;
//            vertices[numVertices].y /= 300;
            numVertices++;
        }
    }

    file.close();
}

void scaleAndTranslate(FloatType2D vertices[], int numVertices) {
    // Calculate object bounds
    FloatType2D minCoords = {FLT_MAX, FLT_MAX};
    FloatType2D maxCoords = {-FLT_MAX, -FLT_MAX};
    for (int i = 0; i < numVertices; ++i) {
        minCoords.x = std::min(minCoords.x, vertices[i].x);
        minCoords.y = std::min(minCoords.y, vertices[i].y);
        maxCoords.x = std::max(maxCoords.x, vertices[i].x);
        maxCoords.y = std::max(maxCoords.y, vertices[i].y);
    }

    // Calculate scale factors and translation values
    FloatType2D scaleFactors = {2.0f / (maxCoords.x - minCoords.x), 2.0f / (maxCoords.y - minCoords.y)};
    FloatType2D translation = {-(maxCoords.x + minCoords.x) / 2.0f, -(maxCoords.y + minCoords.y) / 2.0f};

    // Apply transformation to object coordinates
    for (int i = 0; i < numVertices; ++i) {
        vertices[i].x = (vertices[i].x + translation.x) * scaleFactors.x;
        vertices[i].y = (vertices[i].y + translation.y) * scaleFactors.y;
    }
}


//----------------------------------------------------------------------------

void init( void )
{
    ColorType3D colors[NVERTICES];
    FloatType2D vertices[NVERTICES];
    int numVertices = 0;
    GLuint vao[1], buffer, program, location1, location2;

    // Read object description from file
    readFile("/Users/Z003YW4/github.com/fundamentals-of-computer-graphics/assignment2a/src/cow-nonormals.obj", vertices, colors, numVertices);

    // needed for .obj files
    scaleAndTranslate(vertices, numVertices);

    // Create and bind a vertex array object
    glGenVertexArrays( 1, vao );
    glBindVertexArray( vao[0] );

    // Create and initialize a buffer object large enough to hold both vertex position and color data
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(colors), vertices, GL_STATIC_DRAW );
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
    
    // Define the names of the shader files
    std::stringstream vshader, fshader;
    vshader << SRC_DIR << "/vshader2a.glsl";
    fshader << SRC_DIR << "/fshader2a.glsl";
    
    // Load the shaders and use the resulting shader program
    program = InitShader( vshader.str().c_str(), fshader.str().c_str() );
    
    // Determine locations of the necessary attributes and matrices used in the vertex shader
    location1 = glGetAttribLocation( program, "vertex_position" );
    glEnableVertexAttribArray( location1 );
    glVertexAttribPointer( location1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    location2 = glGetAttribLocation( program, "vertex_color" );
    glEnableVertexAttribArray( location2 );
    glVertexAttribPointer( location2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)) );
    m_location = glGetUniformLocation( program, "M");
    
    // Define static OpenGL state variables
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white, opaque background
    
    // Define some GLFW cursors (in case you want to dynamically change the cursor's appearance)
    // If you want, you can add more cursors, or even define your own cursor appearance
    arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
}

//----------------------------------------------------------------------------

int main(int argc, char** argv) {

    int i;
    GLFWwindow* window;

    // Define the error callback function
    glfwSetErrorCallback(error_callback);

    // Initialize GLFW (performs platform-specific initialization)
    if (!glfwInit()) exit(EXIT_FAILURE);

    // Ask for OpenGL 3.2
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Use GLFW to open a window within which to display your graphics
	window = glfwCreateWindow(window_width, window_height, "HW2a", NULL, NULL);

    // Verify that the window was successfully created; if not, print error message and terminate
    if (!window)
	{
        printf("GLFW failed to create window; terminating\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); // makes the newly-created context current

    // Load all OpenGL functions (needed if using Windows)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("gladLoadGLLoader failed; terminating\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSwapInterval(1);  // tells the system to wait for the rendered frame to finish updating before swapping buffers; can help to avoid tearing

    // Define the keyboard callback function
    glfwSetKeyCallback(window, key_callback);
    // Define the mouse button callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Define the mouse motion callback function
    glfwSetCursorPosCallback(window, cursor_pos_callback);

	// Create the shaders and perform other one-time initializations
	init();

	// event loop
    while (!glfwWindowShouldClose(window)) {

        // fill/re-fill the window with the background color
		glClear( GL_COLOR_BUFFER_BIT );

        // define/re-define the modelview matrix. In this template, we define M to be the identity matrix; you will need define M according to the user's actions.
        updateMatrix();

        // sanity check that your matrix contents are what you expect them to be
        if (DEBUG_ON) printf("M = [%f %f %f %f\n     %f %f %f %f\n     %f %f %f %f\n     %f %f %f %f]\n",M[0],M[4],M[8],M[12], M[1],M[5],M[9],M[13], M[2],M[6],M[10],M[14], M[3],M[7],M[11],M[15]);

        glUniformMatrix4fv( m_location, 1, GL_FALSE, M );   // send the updated model transformation matrix to the GPU
		glDrawArrays( GL_TRIANGLES, 0, NVERTICES );    // draw a triangle between the first vertex and each successive vertex pair in the hard-coded model
		glFlush();	// ensure that all OpenGL calls have executed before swapping buffers

        glfwSwapBuffers(window);  // swap buffers
        glfwWaitEvents(); // wait for a new event before re-drawing

	} // end graphics loop

	// Clean up
	glfwDestroyWindow(window);
	glfwTerminate();  // destroys any remaining objects, frees resources allocated by GLFW
	exit(EXIT_SUCCESS);

} // end main


