# Assignment 0
This is a program to render a scene containing ellipses and spheres in 3D space.

# Building
`$ make`

# Usage
To run individual files:

`$ ./raytracer1a <path/to/input_file>`

To run all the provided examples:

`$ ./raytracer1a.sh`

# Input File Format
Sample input files are provided in this repo.
- example.txt
- example2.txt
- example3.txt
- example4.txt
- example5.txt
- ellipse.txt
- showcase.txt

### General Notes
- The RGB scale is from 0-1.
- Horizontal field of view accepts the value v in degrees, not radians.

### Header
This section defines the scene's general viewing parameters. The individual values can appear in any order, but this section must be at the top.
```
imsize width height
eye x y z
viewdir x y z
hfov v
updir x y z
bkgcolor R G B
```
### Body
This section defines the objects within the scene and their colors. `mtlcolor` must come first followed by the objects that are that color in any order. This section must come last.
```
mtlcolor x y z
sphere x y z r
ellipse x y z rx ry rz

mtlcolor x y z
sphere x y z r
ellipse x y z rx ry rz
```
# Docs
**NOTE: This documentation section is taking the place of explanatory comments baked within my code in order to stay in line with the [industry standard](https://bpoplauschi.github.io/2021/01/20/Clean-Code-Comments-by-Uncle-Bob-part-2.html) on code commenting.**

### types.h
```c
#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_TYPES_H

// Vector3 definition, also used for 3D points
typedef struct {
    float x;
    float y;
    float z;
} Vector3;

// Ray definition
typedef struct {
    Vector3 direction;
    Vector3 origin;
} Ray;

// Field of view definition
typedef struct {
    float h;
    float v;
} FieldOfView;

// Image size definition
typedef struct {
    int width;
    int height;
} ImSize;

// RGBColor definition
typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGBColor;

// Parallel definition
typedef struct {
    float frustumWidth;
} Parallel;

// Sphere definition
typedef struct {
    Vector3 center;
    float radius;
    int mtlColorIdx;
} Sphere;

// Ellipse definition
typedef struct {
    Vector3 center;
    Vector3 radius;
    int mtlColorIdx;
} Ellipse;

// Scene definition
typedef struct {
    Vector3 eye;
    Vector3 viewDir;
    Vector3 upDir;
    FieldOfView fov;
    ImSize imSize;
    RGBColor bkgColor;
    Parallel parallel;
    RGBColor* mtlColors;
    int mtlColorCount;
    Sphere* spheres;
    int sphereCount;
    Ellipse* ellipses;
    int ellipseCount;
} Scene;

// Viewing window definition
typedef struct {
    float width;
    float height;
    Vector3 ul;
    Vector3 ur;
    Vector3 ll;
    Vector3 lr;
} ViewingWindow;

// View parameters definition
typedef struct {
    Vector3 w;
    Vector3 n;
    Vector3 u;
    Vector3 v;
    float aspectRatio;
    float d;
    ViewingWindow viewingWindow;
    Vector3 dh;
    Vector3 dv;
} ViewParameters;

#endif
```

### input.h
```c
#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUT_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <_ctype.h>
#include "types.h"
#include <math.h>

#define MAX_LINE_COUNT 500 // Max number of lines
#define MAX_WORDS_PER_LINE 8 // This will wrap if they have more than this many words in a line and cause weird behavior
#define MAX_LINE_LENGTH 50 // Max length of the line
#define KEYWORD_COUNT 10 // The total number of keywords accepted by the program

// Function to validate arguments
void checkArgs(int argc, char* argv[]) {
    // If an input file isn't passed in, or too many args are passed in then print an error
    if (argc != 2) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./raytracer1a <path/to/input_file>`");
        exit(-1);
    }
    // Make sure they use the correct executable name
    if (strcmp(argv[0], "./raytracer1a") != 0) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./raytracer1a <path/to/input_file>`");
        exit(-1);
    }
}

// Function to read a single line from the file
char** readLine(char* line, char** wordsInLine) {
    // Whitespace delimiters
    char* delimiters = " \t\n";
    // Split the string based on the delimiters
    char* token = strtok(line, delimiters);
    int wordIdx = 0;

    // Loop through the line
    while (token != NULL && wordIdx < MAX_WORDS_PER_LINE) {
        // Remove trailing whitespace
        size_t length = strlen(token);
        while (length > 0 && isspace(token[length - 1])) {
            length--;
        }

        // If there are words in the line
        if (length > 0) {
            // Then allocate memory to read them into the array
            wordsInLine[wordIdx] = (char *) malloc(strlen(token) + 1);
            // Handle memory allocation error
            if (wordsInLine[wordIdx] == NULL) {
                fprintf(stderr, "Memory allocation error for word in line.");
                exit(-1);
            }
        }
        // Copy the token into the array
        strcpy(wordsInLine[wordIdx], token);
        // Get the next token and increment
        token = strtok(NULL, delimiters);
        wordIdx++;
    }
    // Null pointer to denote the end of the array
    wordsInLine[wordIdx] = NULL;

    // remove newline character from the last word if present
    if (wordIdx > 0) {
        // Get the length of the last word
        size_t lastWordLength = strlen(wordsInLine[wordIdx - 1]);
        // If the last word is a new line, replace it with a \0
        if (lastWordLength > 0 && wordsInLine[wordIdx - 1][lastWordLength - 1] == '\n') {
            wordsInLine[wordIdx - 1][lastWordLength - 1] = '\0';
        }
    }

    return wordsInLine;
}

// Helper function to determine if a word is a keyword or not
bool isKeyword(const char* target) {
    // Allocate our keywords as read only so we don't have to free them later
    char* keywords[KEYWORD_COUNT] = {"eye", "viewdir", "updir", "hfov", "imsize", "bkgcolor", "mtlcolor", "sphere", "parallel", "ellipse"};
    // Loop through the keywords
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        // Compare the word to the keyword
        if (strcmp(target, keywords[i]) == 0) {
            // If it matches one of the keywords, return true
            return true;
        }
    }
    // If it matches none of the keywords, return false
    return false;
}

// Function to read the input file
char*** readInputFile(char* inputFileName) {
    // Initialize as NULL to avoid seg faults
    char*** inputFileWordsByLine = NULL;

    // Open the file
    FILE* inputFilePtr;
    inputFilePtr = fopen(inputFileName, "r");

    // Check that the file was valid
    if (inputFilePtr != NULL) {
        // Allocate the memory needed to read the file
        inputFileWordsByLine = malloc(MAX_LINE_COUNT * sizeof(char**));

        // Handle memory allocation error
        if (inputFileWordsByLine == NULL) {
            fprintf(stderr, "Memory allocation error with reading the input file lines.");
            exit(-1);
        }

        // Create a current line array
        char currentLine[MAX_LINE_LENGTH];
        int line = 0;
        // Re-allocate the memory to include the next line and check that the next line is there
        while (
                (inputFileWordsByLine[line] = malloc(MAX_WORDS_PER_LINE * sizeof(char*))) != NULL &&
                fgets(currentLine, MAX_LINE_LENGTH, inputFilePtr) != NULL
        ) {
            // Ignore new line and null
            if (currentLine[0] == '\n' || currentLine[0] == '\0') {
                continue;
            }
            // Check for too many lines error
            if (line > MAX_LINE_COUNT) {
                fprintf(stderr, "Invalid file format. Too many lines.");
                exit(-1);
            }
            // Read the line
            readLine(currentLine, inputFileWordsByLine[line]);
            // Handle if a non-keyword is passed in
            if (!isKeyword(inputFileWordsByLine[line][0])) {
                fprintf(stderr, "Invalid keyword in input file: %s", inputFileWordsByLine[line][0]);
                exit(-1);
            }
            line++;
        }
    } else {
        // Handle if the file is invalid
        fprintf(stderr, "Unable to open the input file specified.");
        exit(-1);
    }

    // Close the input file
    fclose(inputFilePtr);
    return inputFileWordsByLine;
}

// Function to free the previously allocated memory
void freeInputFileWordsByLine(char*** inputFileWordsByLine) {
    for (int i = 0; i < MAX_LINE_COUNT; i++) {
        if (inputFileWordsByLine[i] != NULL) {
            for (int j = 0; j < MAX_WORDS_PER_LINE; j++) {
                if (inputFileWordsByLine[i][j] != NULL) {
                    free(inputFileWordsByLine[i][j]);
                }
            }
            free(inputFileWordsByLine[i]);
        }
    }
    free(inputFileWordsByLine);
}


// Helper function to convert a string to a float
float convertStringToFloat(char* s) {
    char *end;
    float result = strtof(s, &end);
    // Check if conversion worked and handle the error if not
    if (*end == '\0') {
        return result;
    } else {
        fprintf(stderr, "Conversion failed. Invalid float value: %s\n", s);
        exit(-1);
    }
}

// Helper function to convert a string to an int
int convertStringToInt(char* s) {
    char* end;
    int i = (int) strtol(s, &end, 10);
    // Check if conversion worked and handle the error if not
    if (strcmp("", end) != 0 && *end != 0) {
        fprintf(stderr, "Improper file format. Invalid integer value: %s", s);
        exit(-1);
    }
    return i;
}

// Helper function to convert a float to an unsigned char
unsigned char convertFloatToUnsignedChar(float normalizedValue) {
    // Handle the boundaries
    if (normalizedValue < 0.0) {
        normalizedValue = 0.0f;
    } else if (normalizedValue > 1.0) {
        normalizedValue = 1.0f;
    }
    // Convert
    unsigned char result = (unsigned char)(normalizedValue * 255.0);
    return result;
}

// Function to read the input file scene set up header into a Scene object
void readSceneSetup(
        char*** inputFileWordsByLine,
        int* line,
        Scene* scene
) {
    // Start with the header, which is everything before the first mtlcolor
    while (inputFileWordsByLine[*line][0] != NULL && strcmp(inputFileWordsByLine[*line][0], "mtlcolor") != 0) {
        if (strcmp(inputFileWordsByLine[*line][0], "eye") == 0) {
            // Read eye in
            scene->eye.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->eye.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->eye.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "viewdir") == 0) {
            // Read viewdir in
            scene->viewDir.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->viewDir.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->viewDir.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "updir") == 0) {
            // Read updir in
            scene->upDir.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->upDir.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->upDir.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "hfov") == 0) {
            // Read fov in
            scene->fov.h = convertStringToFloat(inputFileWordsByLine[*line][1]) * (float) M_PI / 180.0f; // convert to radians
        } else if (strcmp(inputFileWordsByLine[*line][0], "imsize") == 0) {
            // Read imsize in
            scene->imSize.width = convertStringToInt(inputFileWordsByLine[*line][1]);
            scene->imSize.height = convertStringToInt(inputFileWordsByLine[*line][2]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "bkgcolor") == 0) {
            // Read bkgColor in
            scene->bkgColor.red = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][1]));
            scene->bkgColor.green = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][2]));
            scene->bkgColor.blue = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][3]));
        } else if (strcmp(inputFileWordsByLine[*line][0], "parallel") == 0) {
            // Read parallel in
            scene->parallel.frustumWidth = convertStringToFloat(inputFileWordsByLine[*line][1]);
        }
        (*line)++;
    }
}

// Function to read the input file scene objects into a Scene object
void readSceneObjects(char*** inputFileWordsByLine, int* line, Scene* scene) {
    // Read the body of the file, everything including and past the first mtlcolor
    while (inputFileWordsByLine[*line][0] != NULL) {
        if (strcmp(inputFileWordsByLine[*line][0], "mtlcolor") == 0) {
            // Reallocate memory for each new mtlcolor
            RGBColor* newMtlColors = (RGBColor*) realloc((*scene).mtlColors, ((*scene).mtlColorCount + 1) * sizeof(RGBColor));
            // Handle memory allocation error
            if (newMtlColors == NULL) {
                fprintf(stderr, "Memory allocation failed for material color.");
                exit(-1);
            }
            (*scene).mtlColors = newMtlColors;
            // Set the values after converting to unsigned char
            (*scene).mtlColors[(*scene).mtlColorCount].red = convertFloatToUnsignedChar(
                    convertStringToFloat(inputFileWordsByLine[*line][1]));
            (*scene).mtlColors[(*scene).mtlColorCount].green = convertFloatToUnsignedChar(
                    convertStringToFloat(inputFileWordsByLine[*line][2]));
            (*scene).mtlColors[(*scene).mtlColorCount].blue = convertFloatToUnsignedChar(
                    convertStringToFloat(inputFileWordsByLine[*line][3]));

            // Read all the spheres and ellipses for a given mtlcolor
            int objectLine = (*line) + 1;
            // Go until we reach another mtlcolor
            while (inputFileWordsByLine[objectLine][0] != NULL &&
                   strcmp(inputFileWordsByLine[objectLine][0], "mtlcolor") != 0) {
                // If it's a sphere
                if (strcmp(inputFileWordsByLine[objectLine][0], "sphere") == 0) {
                    // Reallocate the array to include the new sphere
                    Sphere* newSpheres = (Sphere*) realloc((*scene).spheres, ((*scene).sphereCount + 1) * sizeof(Sphere));
                    // Handle memory allocation error
                    if (newSpheres == NULL) {
                        fprintf(stderr, "Memory allocation failed for sphere.");
                        exit(-1);
                    }
                    (*scene).spheres = newSpheres;
                    // Set the sphere position and radius after converting to floats
                    Vector3 spherePosition = {
                            .x = convertStringToFloat(inputFileWordsByLine[objectLine][1]),
                            .y = convertStringToFloat(inputFileWordsByLine[objectLine][2]),
                            .z = convertStringToFloat(inputFileWordsByLine[objectLine][3])
                    };
                    (*scene).spheres[(*scene).sphereCount].center = spherePosition;
                    (*scene).spheres[(*scene).sphereCount].radius = convertStringToFloat(inputFileWordsByLine[objectLine][4]);
                    (*scene).spheres[(*scene).sphereCount].mtlColorIdx = (*scene).mtlColorCount;
                    // Update the sphere count
                    (*scene).sphereCount++;
                } else if (strcmp(inputFileWordsByLine[objectLine][0], "ellipse") == 0) {
                    // If it's an ellipse
                    // Reallocate the array to include the new sphere
                    Ellipse* newEllipses = (Ellipse *) realloc((*scene).ellipses, ((*scene).ellipseCount + 1) * sizeof(Ellipse));
                    // Handle memory allocation error
                    if (newEllipses == NULL) {
                        fprintf(stderr, "Memory allocation failed for ellipse.");
                        exit(-1);
                    }
                    (*scene).ellipses = newEllipses;
                    // Set the ellipse position and radius after converting to floats
                    Vector3 ellipseCenter = {
                            .x = convertStringToFloat(inputFileWordsByLine[objectLine][1]),
                            .y = convertStringToFloat(inputFileWordsByLine[objectLine][2]),
                            .z = convertStringToFloat(inputFileWordsByLine[objectLine][3])
                    };
                    Vector3 ellipseRadius = {
                            .x = convertStringToFloat(inputFileWordsByLine[objectLine][4]),
                            .y = convertStringToFloat(inputFileWordsByLine[objectLine][5]),
                            .z = convertStringToFloat(inputFileWordsByLine[objectLine][6])
                    };
                    (*scene).ellipses[(*scene).ellipseCount].center = ellipseCenter;
                    (*scene).ellipses[(*scene).ellipseCount].radius = ellipseRadius;
                    (*scene).ellipses[(*scene).ellipseCount].mtlColorIdx = (*scene).mtlColorCount;
                    // Update the ellipse count
                    (*scene).ellipseCount++;
                }
                objectLine++;
            }
            (*scene).mtlColorCount++;
            *line = objectLine;
        } else {
            (*line)++;
        }
    }
}

// Helper function to print the scene
// Used to validate the file has been read in correctly
void printInput(Scene scene) {
    printf("--------------------SCENE--------------------\n");
    printf("eye: %lf %lf %lf\n", scene.eye.x, scene.eye.y, scene.eye.z);
    printf("viewdir: %lf %lf %lf\n", scene.viewDir.x, scene.viewDir.y, scene.viewDir.z);
    printf("updir: %lf %lf %lf\n", scene.upDir.x, scene.upDir.y, scene.upDir.z);
    printf("hfov: %lf\n", scene.fov.h);
    printf("imsize: %d %d\n", scene.imSize.width, scene.imSize.height);
    printf("bkgcolor: %d %d %d\n", scene.bkgColor.red, scene.bkgColor.green, scene.bkgColor.blue);
    printf("parallel: %lf\n", scene.parallel.frustumWidth);
    if (scene.mtlColors != NULL) {
        for (int mtlColorIdx = 0; mtlColorIdx < scene.mtlColorCount; mtlColorIdx++) {
            printf("mtlcolor: %d %d %d\n", scene.mtlColors[mtlColorIdx].red, scene.mtlColors[mtlColorIdx].green, scene.mtlColors[mtlColorIdx].blue);
            for (int sphereIdx = 0; sphereIdx < scene.sphereCount; sphereIdx++) {
                if (scene.spheres[sphereIdx].mtlColorIdx == mtlColorIdx) {
                    printf("sphere: %lf %lf %lf %lf\n", scene.spheres[sphereIdx].center.x, scene.spheres[sphereIdx].center.y, scene.spheres[sphereIdx].center.z, scene.spheres[sphereIdx].radius);
                }
            }
            for (int ellipseIdx = 0; ellipseIdx < scene.ellipseCount; ellipseIdx++) {
                if (scene.ellipses[ellipseIdx].mtlColorIdx == mtlColorIdx) {
                    printf(
                            "ellipse: %lf %lf %lf %lf %lf %lf\n",
                            scene.ellipses[ellipseIdx].center.x,
                            scene.ellipses[ellipseIdx].center.y,
                            scene.ellipses[ellipseIdx].center.z,
                            scene.ellipses[ellipseIdx].radius.x,
                            scene.ellipses[ellipseIdx].radius.y,
                            scene.ellipses[ellipseIdx].radius.z
                    );
                }
            }
        }
    }
    printf("---------------------------------------------\n\n");
}

// Function to free the previously allocated mtlcolors, spheres, and ellipses
void freeInput(Scene scene) {
    if (scene.mtlColors != NULL) {
        free(scene.mtlColors);
    }
    if (scene.spheres != NULL) {
        free(scene.spheres);
    }
    if (scene.ellipses != NULL) {
        free(scene.ellipses);
    }
}

#endif
```


### output.h
```c
#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_OUTPUT_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_OUTPUT_H

#include "render.h"
#include "stringhelper.h"

#define MAX_INPUT_FILE_NAME_LENGTH 100 // Max length of the name of the input file
#define OUTPUT_FILE_SUFFIX ".ppm" // Output file type
#define MAGIC_NUMBER "P3" // Magic number for the PPM header
#define MAX_COLOR_COMPONENT_VALUE "255" // 255 because we're using RGB color space
#define MAX_PIXELS_ON_LINE 5 // Keep the lines of our PPM file short

// Function to open the output file
FILE* openOutputFile(char* inputFileName) {
    // Only accept .txt files
    if (!endsWith(inputFileName, ".txt")) {
        fprintf(stderr, "Incorrect input file format. Input file must be a '.txt' file.");
        exit(-1);
    }
    // Define the output file name to be the same as the input file name without the .txt ending
    char outputFileName[MAX_INPUT_FILE_NAME_LENGTH + 3];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", substr(inputFileName, 0, strlen(inputFileName) - 4), OUTPUT_FILE_SUFFIX);

    // Open the output file and return the pointer to it
    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");
    return outputFilePtr;
}

// Function to write the PPM Header to the output file
void writeHeader(FILE* outputFilePtr, int width, int height) {
    // P3 x y 255
    fprintf(outputFilePtr, "%s\n%d %d\n%s\n", MAGIC_NUMBER, width, height, MAX_COLOR_COMPONENT_VALUE);
}

// Function to make sure the lines of the PPM file are short
void enforceMaxPixelsOnLine(FILE* outputFilePtr, int x, int width) {
    // If we've reached the max pixels on the line
    if ((x % (MAX_PIXELS_ON_LINE) == (MAX_PIXELS_ON_LINE - 1)) || x == width - 1) {
        // Then start a new line
        fprintf(outputFilePtr, "\n");
    } else {
        // Otherwise use a tab between pixels for readability
        fprintf(outputFilePtr, "\t");
    }
}

// Function to write a pixel to the file
void writePixel(FILE* outputFilePtr, RGBColor pixelColor, int x, int width) {
    // Write the pixel and enforce the max pixels on line rule
    fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
    enforceMaxPixelsOnLine(outputFilePtr, x, width);
}

#endif
```

### render.h
```c
#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_RENDER_H

#include <float.h>

float magnitude(Vector3 v) {
    return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

Vector3 normalize(Vector3 v) {
    float m = magnitude(v);
    if (m != 0.0) {
        return (Vector3) {
                .x = v.x / m,
                .y = v.y / m,
                .z = v.z / m
        };
    } else {
        // What to do if the vector has 0 magnitude??
        return v;
    }
}

Vector3 multiply(Vector3 v, float s) {
    return (Vector3) {
            .x = v.x * s,
            .y = v.y * s,
            .z = v.z * s
    };
}

Vector3 cross(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = (a.y * b.z) - (b.y * a.z),
            .y = (a.z * b.x) - (b.z* a.x),
            .z = (a.x * b.y) - (b.x * a.y)
    };
}

float dot(Vector3 a, Vector3 b) {
    return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

Vector3 add(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = a.x + b.x,
            .y = a.y + b.y,
            .z = a.z + b.z
    };
}

Vector3 subtract(Vector3 a, Vector3 b) {
    return (Vector3) {
            .x = a.x - b.x,
            .y = a.y - b.y,
            .z = a.z - b.z
    };
}

Vector3 divide(Vector3 v, float c) {
    return (Vector3) {
            .x = v.x / c,
            .y = v.y /c,
            .z = v.z / c
    };
}

void printVector(Vector3 v) {
    printf("{x: %lf, y: %lf, z: %lf}\n", v.x, v.y, v.z);
}

void printViewParameters(ViewParameters viewParameters) {
    printf("---------------VIEW PARAMETERS---------------\n");
    printf("w: ");
    printVector(viewParameters.w);
    printf("u: ");
    printVector(viewParameters.u);
    printf("v: ");
    printVector(viewParameters.v);
    printf("Aspect ratio: %lf\n", viewParameters.aspectRatio);
    printf("d: %lf\n", viewParameters.d);
    printf("width: %lf\n", viewParameters.viewingWindow.width);
    printf("height: %lf\n", viewParameters.viewingWindow.height);
    printf("ul: ");
    printVector(viewParameters.viewingWindow.ul);
    printf("ur: ");
    printVector(viewParameters.viewingWindow.ur);
    printf("ll: ");
    printVector(viewParameters.viewingWindow.ll);
    printf("lr: ");
    printVector(viewParameters.viewingWindow.lr);
    printf("---------------------------------------------\n");
}

void setViewingWindow(Scene scene, ViewParameters* viewParameters) {
    float halfWidth = viewParameters->viewingWindow.width/2;
    float halfHeight = viewParameters->viewingWindow.height/2;
    Vector3 widthTimesHorizontal = multiply(viewParameters->u, halfWidth);
    Vector3 heightTimesVertical = multiply(viewParameters->v, halfHeight);
    Vector3 eyePlusViewVector = add(scene.eye, multiply(viewParameters->n, viewParameters->d));
    Vector3 perspectiveMinusDimensions = subtract(eyePlusViewVector, widthTimesHorizontal);
    Vector3 perspectivePlusDimensions = add(eyePlusViewVector, widthTimesHorizontal);

    viewParameters->viewingWindow.ul = add(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ur = add(perspectivePlusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.ll = subtract(perspectiveMinusDimensions, heightTimesVertical);
    viewParameters->viewingWindow.lr = subtract(perspectivePlusDimensions, heightTimesVertical);

    viewParameters->dh = divide(subtract(viewParameters->viewingWindow.ur, viewParameters->viewingWindow.ul), ((float) scene.imSize.width - 1));
    viewParameters->dv = divide(subtract(viewParameters->viewingWindow.ll, viewParameters->viewingWindow.ul), ((float) scene.imSize.height - 1));
}

Ray createRay(Scene scene, ViewParameters viewParameters, int x, int y) {
    Vector3 viewingWindowLocation = add(
            add(
                    viewParameters.viewingWindow.ul,
                    multiply(
                            viewParameters.dh,
                            (float) x
                        )
                ),
                multiply(viewParameters.dv, (float) y)
    );

    if (scene.parallel.frustumWidth > 0) {
        return (Ray) {
            .origin = multiply(viewingWindowLocation, -1),
            .direction = normalize(scene.viewDir)
        };
    } else {
        return (Ray) {
                .origin = scene.eye,
                .direction = normalize(subtract(viewingWindowLocation, scene.eye))
        };
    }
}

// Function to determine the pixel color based on collision detection with the ray and objects
RGBColor getPixelColor(Ray ray, Scene scene, int y, char* argv) {
    // Initialize with a large value
    float closestIntersection = FLT_MAX;
    // Variables to track which object is the closest
    bool closestIsSphere = false;
    int closestSphereIdx = -1;
    int closestEllipseIdx = -1;

    // Normalize the ray dir and subtract the ray origin
    Vector3 rayDir = normalize(subtract(ray.direction, ray.origin));
    // Loop through each sphere
    for (int sphereIdx = 0; sphereIdx < scene.sphereCount; sphereIdx++) {
        Sphere sphere = scene.spheres[sphereIdx];
        // Define the quadratic equation for a sphere
        float A = dot(rayDir, rayDir);
        float B = 2 * dot(rayDir, subtract(ray.origin, sphere.center));
        float C = dot(subtract(ray.origin, sphere.center), subtract(ray.origin, sphere.center)) - sphere.radius * sphere.radius;

        // Isolate the discriminant
        float discriminant = B * B - 4 * A * C;

        // Check for solutions to the quadratic
        if (discriminant >= 0) {
            // If there's solutions, get them both
            // t1 or t2 will be 0 if there's only one solution
            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2 * A);
            float t2 = (-B - sqrtDiscriminant) / (2 * A);

            // Determine which of the two collision points is closest and set the corresponding values
            if (t1 >= 0 && t1 < closestIntersection) {
                closestIntersection = t1;
                closestSphereIdx = sphereIdx;
                closestIsSphere = true;
            }
            if (t2 >= 0 && t2 < closestIntersection) {
                closestIntersection = t2;
                closestSphereIdx = sphereIdx;
                closestIsSphere = true;
            }
        }
    }

    // Loop through each ellipse
    for (int ellipseIdx = 0; ellipseIdx < scene.ellipseCount; ellipseIdx++) {
        Ellipse ellipse = scene.ellipses[ellipseIdx];

        // Define the quadratic equation for an ellipse
        float A = (rayDir.x * rayDir.x) / (ellipse.radius.x * ellipse.radius.x)
                   + (rayDir.y * rayDir.y) / (ellipse.radius.y * ellipse.radius.y)
                   + (rayDir.z * rayDir.z) / (ellipse.radius.z * ellipse.radius.z);


        float B = 2 * ((rayDir.x * (ray.origin.x - ellipse.center.x)) / (ellipse.radius.x * ellipse.radius.x)
                        + (rayDir.y * (ray.origin.y - ellipse.center.y)) / (ellipse.radius.y * ellipse.radius.y)
                        + (rayDir.z * (ray.origin.z - ellipse.center.z)) / (ellipse.radius.z * ellipse.radius.z));

        float C = ((ray.origin.x - ellipse.center.x) * (ray.origin.x - ellipse.center.x)) / (ellipse.radius.x * ellipse.radius.x)
                   + ((ray.origin.y - ellipse.center.y) * (ray.origin.y - ellipse.center.y)) / (ellipse.radius.y * ellipse.radius.y)
                   + ((ray.origin.z - ellipse.center.z) * (ray.origin.z - ellipse.center.z)) / (ellipse.radius.z * ellipse.radius.z) - 1;


        // Isolate the discriminant
        float discriminant = B * B - 4 * A * C;

        // Check for solutions to the quadratic
        // Only get the outer ring of the ellipse for the showcase to simulate Saturn's rings
        if ((strcmp(argv, "./showcase.txt") == 0 && discriminant >= 0 && discriminant <= 25) || discriminant >= 0) {
          // If there's solutions, get them both
            // t1 or t2 will be 0 if there's only one solution
            float sqrtDiscriminant = sqrtf(discriminant);
            float t1 = (-B + sqrtDiscriminant) / (2 * A);
            float t2 = (-B - sqrtDiscriminant) / (2 * A);

            // Determine which of the two collision points is closest and set the corresponding values
            if (t1 >= 0 && t1 < closestIntersection) {
                closestIntersection = t1;
                closestEllipseIdx = ellipseIdx;
                closestIsSphere = false;
            }
            if (t2 >= 0 && t2 < closestIntersection) {
                closestIntersection = t2;
                closestEllipseIdx = ellipseIdx;
                closestIsSphere = false;
            }
        }
    }

    // If there's a sphere that's the closest
    if (closestSphereIdx != -1 && closestIsSphere) {
        Sphere sphere = scene.spheres[closestSphereIdx];
        // Handle the special coloration for the showcase
        if (strcmp(argv, "./showcase.txt") == 0) {
            if (sphere.center.z == -10) { // saturn
                return (RGBColor) {
                        .red = (y + 15) % 310,
                        .green = (y - 8) % 310,
                        .blue = y % 310,
                };
            } else if (sphere.center.z == -300) { // sun
                return (RGBColor) {
                        .red = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].red,
                        .green = rand() % 255,
                        .blue = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].blue,
                };
            } else if (sphere.center.z == -500) { // neptune
                return (RGBColor) {
                        .red = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].red,
                        .green = (y + scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].green) % 175,
                        .blue = (y + scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].blue) % 215
                };
            } else if (sphere.center.z == -150) { // mars
                return (RGBColor) {
                        .red = ((y + scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].red) / 2) % 225,
                        .green = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].green,
                        .blue = scene.mtlColors[scene.spheres[closestSphereIdx].mtlColorIdx].blue
                };
            } else {
                // Return the color of the sphere
                return scene.mtlColors[sphere.mtlColorIdx];
            }
        } else {
            // Return the color of the sphere
            return scene.mtlColors[sphere.mtlColorIdx];
        }
    } else if (closestEllipseIdx != -1 && !closestIsSphere) {
        // If there's an ellipse that's the closest
        Ellipse ellipse = scene.ellipses[closestEllipseIdx];
        // Handle the special coloration for the showcase
        if (strcmp(argv, "./showcase.txt") == 0) {
            if (ellipse.center.z == -10) {
                return (RGBColor) {
                        .red = (y + scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].red) % 318,
                        .green = (y + scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].green) % 318,
                        .blue = (y + scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx].blue) % 318,
                };
            } else {
                // Return the color of the ellipse
                return scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx];
            }
        } else {
            // Return the color of the ellipse
            return scene.mtlColors[scene.ellipses[closestEllipseIdx].mtlColorIdx];
        }
    } else {
        // Return stars in the background for the showcase
        if (strcmp(argv, "./showcase.txt") == 0) {
            return rand() % 10000 > 1 ? scene.bkgColor : (RGBColor) {
                    .red = 255,
                    .green = 255,
                    .blue = 255
            };
        } else {
            // Return the background color
            return scene.bkgColor;
        }
    }
}

#endif
```

### Main.c
```c
void render(FILE* outputFilePtr, Scene scene, ViewParameters viewParameters, char* argv) {
    // Loop through every pixel in the image
    for (int y = 0; y < scene.imSize.height; y++) {
        for (int x = 0; x < scene.imSize.width; x++) {
            // Create a ray going from the correct origin in the correct direction
            Ray ray = createRay(scene, viewParameters, x, y);
            // Write the pixel after getting the correct pixel color
            writePixel(outputFilePtr, getPixelColor(ray, scene, y, argv), x, scene.imSize.width);
        }
    }
}

int main(int argc, char* argv[]) {
    // Check the program arguments
    checkArgs(argc, argv);

    // Read the input file and return a list of sentences containing characters
    char*** inputFileWordsByLine = readInputFile(argv[1]);

    // Default the scene because the arrays must be instantiated 
    // with NULL to avoid seg faults
    Scene scene = {
            .eye = { .x = 0, .y = 0, .z = 0 },
            .viewDir = { .x = 0, .y = 0, .z = 0 },
            .upDir = { .x = 0, .y = 0, .z = 0 },
            .fov = { .h = 0 },
            .imSize = { .width = 0, .height = 0 },
            .bkgColor = { .red = 0, .green = 0, .blue = 0 },
            .parallel = { .frustumWidth = 0 },
            .mtlColors = NULL,
            .mtlColorCount = 0,
            .spheres = NULL,
            .sphereCount = 0,
            .ellipses = NULL,
            .ellipseCount = 0
    };

    // The line variable is shared between reading the scene set up
    // and the scene objects.
    int line = 0;
    // Read the header from the file and store in the scene instance
    readSceneSetup(inputFileWordsByLine, &line, &scene);
    // Read the objects from the file and store in the scene instance
    readSceneObjects(inputFileWordsByLine, &line, &scene);

    // De-allocate the stringly typed input that we read earlier 
    // because we now have access to it in the scene instance
    freeInputFileWordsByLine(inputFileWordsByLine);

    // Initialize what we can for the view parameters
    ViewParameters viewParameters = {
            // Standard convention is for w=-viewdir
            .w = normalize(multiply(scene.viewDir, -1)),
            // Get the first vector in our viewing window by doing viewdir x updir
            .u = normalize(cross(scene.viewDir, scene.upDir)),
            // Get the second vector in our viewing window by doing u x viewdir
            .v = cross(viewParameters.u, normalize(scene.viewDir)),
            // We need the non-negative viewDir to calculate the viewing window corners later
            .n = normalize(scene.viewDir),
            // Arbitrary value for distance
            .d = 1.0f,
            // We need the aspect ratio to get the width and height of the viewing window
            .aspectRatio = (float) scene.imSize.width / (float) scene.imSize.height,
            // Define the viewing window instance
            .viewingWindow = {
                    // Get the width by doing 2dtan(θh/2)
                    .width = 2 * viewParameters.d * tanf(scene.fov.h / 2),
                    // Get the height by taking the width and dividing by the aspect ratio
                    .height = 2 * viewParameters.d * (tanf(scene.fov.h / 2) / viewParameters.aspectRatio),
            }
    };
    // Set the rest of the viewing parameters
    setViewingWindow(scene, &viewParameters);

    // Open the output file
    FILE* outputFilePtr = openOutputFile(argv[1]);
    // Write the PPM format header
    writeHeader(outputFilePtr, scene.imSize.width, scene.imSize.height);
    // Render the scene
    render(outputFilePtr, scene, viewParameters, argv[1]);

    // De allocate the memory used in the scene
    freeInput(scene);
    // Exit the program
    exit(0);
}
```
