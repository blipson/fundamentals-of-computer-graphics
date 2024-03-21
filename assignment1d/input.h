#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUT_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUT_H

#include "types.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_COUNT 500000
#define MAX_WORDS_PER_LINE 500 // This will wrap if they have more than this many words in a line and cause weird behavior
#define MAX_INPUT_LINE_LENGTH 5000
#define MAX_TEXTURE_LINE_LENGTH 50000
#define MAX_TEXTURE_WORDS_PER_LINE 10000
#define KEYWORD_COUNT 20
#define INITIAL_LIGHT_COUNT 10
#define INITIAL_MTLCOLOR_COUNT 10
#define INITIAL_TEXTURE_COUNT 10
#define INITIAL_NORMAL_COUNT 10
#define INITIAL_SPHERE_COUNT 10
#define INITIAL_ELLIPSOID_COUNT 10
#define INITIAL_VERTEX_COUNT 10000
#define INITIAL_VERTEX_NORMAL_COUNT 10000
#define INITIAL_VERTEX_TEXTURE_COUNT 10000
#define INITIAL_FACE_COUNT 10000

void checkArgs(int argc, char* argv[]) {
    if (argc > 3 || argc < 2) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./raytracer1d [-s:soft shadows] <path/to/input_file>`\n");
        exit(-1);
    }
    if (strcmp(argv[0], "./raytracer1d") != 0 && strcmp(argv[0], "/home/ben/github.com/fundamentals-of-computer-graphics/assignment1d/main") != 0  && strcmp(argv[0], "/Users/Z003YW4/github.com/fundamentals-of-computer-graphics/assignment1d/main") != 0) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./raytracer1c <path/to/input_file>`\n");
        exit(-1);
    }
}

char** readLine(char* line, char** wordsInLine, int maxWordsPerLine) {
    char* delimiters = " \t\n\r";
    char* token = strtok(line, delimiters);
    int wordIdx = 0;

    while (token != NULL && wordIdx < maxWordsPerLine - 1) {
        // Remove trailing whitespace
        size_t length = strlen(token);
        while (length > 0 && isspace(token[length - 1])) {
            length--;
        }

        if (length > 0) {
            wordsInLine[wordIdx] = (char*) malloc(strlen(token) + 1);
            if (wordsInLine[wordIdx] == NULL) {
                fprintf(stderr, "Memory allocation error for word in line.\n");
                exit(-1);
            }
        }
        strcpy(wordsInLine[wordIdx], token);
        token = strtok(NULL, delimiters);
        wordIdx++;
    }
    wordsInLine[wordIdx] = NULL;

    // remove newline character from the last word if present
    if (wordIdx > 0) {
        size_t lastWordLength = strlen(wordsInLine[wordIdx - 1]);
        if (lastWordLength > 0 && wordsInLine[wordIdx - 1][lastWordLength - 1] == '\n') {
            wordsInLine[wordIdx - 1][lastWordLength - 1] = '\0';
        }
    }

    return wordsInLine;
}

bool isKeyword(const char* target) {
    char* keywords[KEYWORD_COUNT] = {
            "eye", "viewdir", "updir", "hfov", "vfov",
            "imsize", "bkgcolor", "mtlcolor", "sphere",
            "parallel", "ellipse", "light", "depthcueing",
            "attlight", "v", "vn", "f", "texture",
            "vt", "bump",
    };
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        if (target == NULL || strcmp(target, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

char*** readInputFile(char* argv[], bool softShadows) {
    char*** inputFileWordsByLine = NULL;
    char* inputFileName = (softShadows) ? argv[2] : argv[1];

    FILE* inputFilePtr = fopen(inputFileName, "r");

    if (inputFilePtr != NULL) {
        inputFileWordsByLine = malloc(MAX_LINE_COUNT * sizeof(char**));
        if (inputFileWordsByLine == NULL) {
            fprintf(stderr, "Memory allocation error with reading the input file lines.\n");
            exit(-1);
        }

        char currentLine[MAX_INPUT_LINE_LENGTH];
        int line = 0;
        while ((inputFileWordsByLine[line] = malloc(MAX_WORDS_PER_LINE * sizeof(char*))) != NULL &&
               fgets(currentLine, MAX_INPUT_LINE_LENGTH, inputFilePtr) != NULL) {
            if (currentLine[0] == '\n' || currentLine[0] == '\0' || currentLine[0] == '\r') {
                continue;
            }
            if (line > MAX_LINE_COUNT) {
                fprintf(stderr, "Invalid file format. Too many lines.\n");
                exit(-1);
            }
            readLine(currentLine, inputFileWordsByLine[line], MAX_WORDS_PER_LINE);
            if (!isKeyword(inputFileWordsByLine[line][0])) {
                fprintf(stderr, "Invalid keyword in input file: %s\n", inputFileWordsByLine[line][0]);
                exit(-1);
            }
            line++;
        }
    } else {
        fprintf(stderr, "Unable to open the input file specified.\n");
        exit(-1);
    }

    fclose(inputFilePtr);
    return inputFileWordsByLine;
}

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

float convertStringToFloat(char* s) {
    char* end;
    float result = strtof(s, &end);
    if (*end == '\0') {
        return result;
    } else {
        fprintf(stderr, "Conversion failed. Invalid float value: %s\n", s);
        exit(-1);
    }
}

int convertStringToInt(char* s) {
    char* end;
    int i = (int) strtol(s, &end, 10);
    if (strcmp("", end) != 0 && *end != 0) {
        fprintf(stderr, "Improper file format. Invalid integer value: %s\n", s);
        exit(-1);
    }
    return i;
}

unsigned char convertFloatToUnsignedChar(float normalizedValue) {
    if (normalizedValue < 0.0f) {
        normalizedValue = 0.0f;
    } else if (normalizedValue > 1.0f) {
        normalizedValue = 1.0f;
    }
    return (unsigned char) (normalizedValue * 255.0f);;
}

float convertUnsignedCharToFloat(unsigned char value) {
    return (float) value;
}

void checkValues(char** line, int expectedNumber, char* type) {
    if (line[expectedNumber + 1] != NULL) {
        fprintf(stderr, "Too many values given for '%s', it expects %d\n", type, expectedNumber);
        exit(-1);
    }
    for (int i = 1; i <= expectedNumber; i++) {
        if (line[i] == NULL) {
            fprintf(stderr, "Too few values given for '%s', it expects %d\n", type, expectedNumber);
            exit(-1);
        }
    }
}

float max(float a, float b) {
    return (a > b) ? a : b;
}

float min(float a, float b) {
    return (a <= b) ? a : b;
}


void readLight(char** const* inputFileWordsByLine, const int* line, Scene* scene, int lightAllocationCount, bool attLight) {
    if (scene->lightCount >= INITIAL_LIGHT_COUNT * lightAllocationCount) {
        lightAllocationCount++;
        Light* newLights = (Light*) realloc(scene->lights, (INITIAL_LIGHT_COUNT * lightAllocationCount) * sizeof(Light));
        if (newLights == NULL) {
            fprintf(stderr, "Memory allocation failed for light\n");
            exit(-1);
        }
        scene->lights = newLights;
    }
    checkValues(inputFileWordsByLine[*line], attLight ? 8 : 5,  attLight ? "attlight" : "light");
    scene->lights[scene->lightCount].position = (Vector3) {
            .x = convertStringToFloat(inputFileWordsByLine[*line][1]),
            .y = convertStringToFloat(inputFileWordsByLine[*line][2]),
            .z = convertStringToFloat(inputFileWordsByLine[*line][3])
    };
    scene->lights[scene->lightCount].pointOrDirectional = convertStringToFloat(inputFileWordsByLine[*line][4]);
    scene->lights[scene->lightCount].intensity = max(min(convertStringToFloat(inputFileWordsByLine[*line][5]), 1), 0); // clamp light intensity to 0-1
    scene->lights[scene->lightCount].constantAttenuation = attLight ? convertStringToFloat(inputFileWordsByLine[*line][6]) : 0.0f;
    scene->lights[scene->lightCount].linearAttenuation = attLight ? convertStringToFloat(inputFileWordsByLine[*line][7]) : 0.0f;
    scene->lights[scene->lightCount].quadraticAttenuation = attLight ? convertStringToFloat(inputFileWordsByLine[*line][8]) : 0.0f;
    scene->lightCount++;
}

void readSceneSetup(
        char*** inputFileWordsByLine,
        int* line,
        Scene* scene,
        bool softShadows
) {
    int lightAllocationCount = 1;
    while (inputFileWordsByLine[*line][0] != NULL &&
        strcmp(inputFileWordsByLine[*line][0], "mtlcolor") != 0 &&
        strcmp(inputFileWordsByLine[*line][0], "v") != 0 &&
        strcmp(inputFileWordsByLine[*line][0], "texture") != 0
    ) {
        if (strcmp(inputFileWordsByLine[*line][0], "eye") == 0) {
            checkValues(inputFileWordsByLine[*line], 3, "eye");
            scene->eye.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->eye.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->eye.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "viewdir") == 0) {
            checkValues(inputFileWordsByLine[*line], 3, "viewdir");
            scene->viewDir.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->viewDir.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->viewDir.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "updir") == 0) {
            checkValues(inputFileWordsByLine[*line], 3, "updir");
            scene->upDir.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->upDir.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->upDir.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "hfov") == 0) {
            checkValues(inputFileWordsByLine[*line], 1, "hfov");
            scene->fov.h = convertStringToFloat(inputFileWordsByLine[*line][1]) * (float) M_PI / 180.0f; // convert to radians
        } else if (strcmp(inputFileWordsByLine[*line][0], "vfov") == 0) {
            checkValues(inputFileWordsByLine[*line], 1, "vfov");
            scene->fov.v = convertStringToFloat(inputFileWordsByLine[*line][1]) * (float) M_PI / 180.0f; // convert to radians
        } else if (strcmp(inputFileWordsByLine[*line][0], "imsize") == 0) {
            checkValues(inputFileWordsByLine[*line], 2, "imsize");
            scene->imSize.width = convertStringToInt(inputFileWordsByLine[*line][1]);
            scene->imSize.height = convertStringToInt(inputFileWordsByLine[*line][2]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "bkgcolor") == 0) {
            checkValues(inputFileWordsByLine[*line], 4, "bkgcolor");
            scene->bkgColor.color.x = convertStringToFloat(inputFileWordsByLine[*line][1]);
            scene->bkgColor.color.y = convertStringToFloat(inputFileWordsByLine[*line][2]);
            scene->bkgColor.color.z = convertStringToFloat(inputFileWordsByLine[*line][3]);
            scene->bkgColor.refractionIndex = convertStringToFloat(inputFileWordsByLine[*line][4]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "parallel") == 0) {
            checkValues(inputFileWordsByLine[*line], 1, "parallel");
            scene->parallel.frustumWidth = convertStringToFloat(inputFileWordsByLine[*line][1]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "light") == 0) {
            readLight(inputFileWordsByLine, line, scene, lightAllocationCount, false);
        } else if (strcmp(inputFileWordsByLine[*line][0], "depthcueing") == 0) {
            scene->depthCueing = (DepthCueing) {
                    .color = (Vector3) {
                            .x = convertStringToFloat(inputFileWordsByLine[*line][1]),
                            .y = convertStringToFloat(inputFileWordsByLine[*line][2]),
                            .z = convertStringToFloat(inputFileWordsByLine[*line][3]),
                    },
                    .min = convertStringToFloat(inputFileWordsByLine[*line][4]),
                    .max = convertStringToFloat(inputFileWordsByLine[*line][5]),
                    .distMin = convertStringToFloat(inputFileWordsByLine[*line][6]),
                    .distMax = convertStringToFloat(inputFileWordsByLine[*line][7]),
            };
        } else if (strcmp(inputFileWordsByLine[*line][0], "attlight") == 0) {
            readLight(inputFileWordsByLine, line, scene, lightAllocationCount, true);
        }
        scene->softShadows = softShadows;
        (*line)++;
    }
}

void readVertex(char** const* inputFileWordsByLine, Scene* scene, int objectLine, int* vertexAllocationCount) {
    if (scene->vertexCount >= INITIAL_VERTEX_COUNT * (*vertexAllocationCount)) {
        (*vertexAllocationCount)++;
        Vector3* newVertexes = (Vector3*) realloc(scene->vertexes, (INITIAL_VERTEX_COUNT * (*vertexAllocationCount)) * sizeof(Vector3));
        if (newVertexes == NULL) {
            fprintf(stderr, "Memory allocation failed for vertexes.");
            exit(-1);
        }
        scene->vertexes = newVertexes;
    }
    checkValues(inputFileWordsByLine[objectLine], 3, "v");
    scene->vertexes[scene->vertexCount] = (Vector3) {
            .x = convertStringToFloat(inputFileWordsByLine[objectLine][1]),
            .y = convertStringToFloat(inputFileWordsByLine[objectLine][2]),
            .z = convertStringToFloat(inputFileWordsByLine[objectLine][3])
    };
    scene->vertexCount++;
}

void readVertexNormal(char** const* inputFileWordsByLine, Scene* scene, int line, int* vertexNormalAllocationCount) {
    if (scene->vertexNormalCount == 0) {
        (*vertexNormalAllocationCount) = 1;
        scene->vertexNormals = (Vector3*) malloc(INITIAL_VERTEX_NORMAL_COUNT * sizeof(Vector3));
    } else if (scene->vertexNormalCount >= INITIAL_VERTEX_NORMAL_COUNT * (*vertexNormalAllocationCount)) {
        (*vertexNormalAllocationCount)++;
        Vector3* newVertexNormals = (Vector3*) realloc(scene->vertexNormals, (INITIAL_VERTEX_NORMAL_COUNT * (*vertexNormalAllocationCount)) * sizeof(Vector3));
        if (newVertexNormals == NULL) {
            fprintf(stderr, "Memory allocation failed for vertex normals.");
            exit(-1);
        }
        scene->vertexNormals = newVertexNormals;
    }
    checkValues(inputFileWordsByLine[line], 3, "vn");
    scene->vertexNormals[scene->vertexNormalCount] = (Vector3) {
            .x = convertStringToFloat(inputFileWordsByLine[line][1]),
            .y = convertStringToFloat(inputFileWordsByLine[line][2]),
            .z = convertStringToFloat(inputFileWordsByLine[line][3])
    };
    scene->vertexNormalCount++;
}

void parseFaceValues(char** objectLine, int faceIdx, const Scene* scene, int objectLineIdx) {
    int faceValueIdx = 0;
    char* faceToken = strsep(&objectLine[faceIdx], "/");
    while (faceToken != NULL) {
        if (faceValueIdx == 0) {
            if (faceIdx == 1) {
                scene->faces[scene->faceCount].v1 = convertStringToInt(faceToken);
            } else if (faceIdx == 2) {
                scene->faces[scene->faceCount].v2 = convertStringToInt(faceToken);
            } else if (faceIdx == 3) {
                scene->faces[scene->faceCount].v3 = convertStringToInt(faceToken);
            }
        } else if (faceValueIdx == 1) {
            if (faceIdx == 1) {
                scene->faces[scene->faceCount].vt1 = convertStringToInt(faceToken);
            } else if (faceIdx == 2) {
                scene->faces[scene->faceCount].vt2 = convertStringToInt(faceToken);
            } else if (faceIdx == 3) {
                scene->faces[scene->faceCount].vt3 = convertStringToInt(faceToken);
            }
        } else if (faceValueIdx == 2) {
            if (faceIdx == 1) {
                scene->faces[scene->faceCount].vn1 = convertStringToInt(faceToken);
            } else if (faceIdx == 2) {
                scene->faces[scene->faceCount].vn2 = convertStringToInt(faceToken);
            } else if (faceIdx == 3) {
                scene->faces[scene->faceCount].vn3 = convertStringToInt(faceToken);
            }
        } else {
            fprintf(stderr, "Too many values for the vertices of the face at %d.\n", objectLineIdx);
        }
        faceToken = strsep(&objectLine[faceIdx], "/");
        faceValueIdx++;
    }
}

PPMImage readPPM(const char *filename) {
    FILE *filePtr = fopen(filename, "rb");
    if (filePtr == NULL) {
        fprintf(stderr, "Error opening file: %s.\n", filename);
        exit(1);
    }

    PPMImage image = (PPMImage) {
        .width = 0,
        .height = 0,
        .maxColor = 0,
        .data = NULL,
    };

    char headerLine[MAX_INPUT_LINE_LENGTH];
    fgets(headerLine, MAX_INPUT_LINE_LENGTH, filePtr);
    char* wordsInHeaderLine[MAX_TEXTURE_WORDS_PER_LINE];
    readLine(headerLine, wordsInHeaderLine, MAX_WORDS_PER_LINE);
    if (strcmp(wordsInHeaderLine[0], "P3") != 0) {
        fprintf(stderr, "Invalid PPM file format.\n");
        exit(1);
    }
    if (wordsInHeaderLine[1] == NULL || wordsInHeaderLine[2] == NULL || wordsInHeaderLine[3] == NULL) {
        fprintf(stderr, "Invalid PPM file header format.\n");
        exit(-1);
    }
    image.width = convertStringToInt(wordsInHeaderLine[1]);
    image.height = convertStringToInt(wordsInHeaderLine[2]);
    image.maxColor = convertStringToInt(wordsInHeaderLine[3]);
    image.data = (RGBColor**) malloc(image.height * sizeof(RGBColor*));
    if (image.data == NULL) {
        fprintf(stderr, "Memory allocation error while reading PPM data.\n");
        exit(1);
    }
    int y = 0;
    char currentLine[MAX_TEXTURE_LINE_LENGTH];
    while (fgets(currentLine, MAX_TEXTURE_LINE_LENGTH, filePtr) != NULL) {
        char* wordsInLine[MAX_TEXTURE_WORDS_PER_LINE];
        readLine(currentLine, wordsInLine, MAX_TEXTURE_WORDS_PER_LINE);
        image.data[y] = (RGBColor*) malloc(image.width * sizeof(RGBColor));
        if (image.data[y] == NULL) {
            fprintf(stderr, "Memory allocation error while reading PPM data.\n");
            exit(1);
        }

        int x = 0;
        int wordIdx = 0;
        while (wordsInLine[wordIdx] != NULL) {
            // todo: handle single value on line. Line breaks inside pixels.
            unsigned char red = convertStringToInt(wordsInLine[wordIdx]);
            wordIdx++;
            if (wordsInLine[wordIdx] == NULL) {
                continue;
            }
            unsigned char green = convertStringToInt(wordsInLine[wordIdx]);
            wordIdx++;
            if (wordsInLine[wordIdx] == NULL) {
                continue;
            }
            unsigned char blue = convertStringToInt(wordsInLine[wordIdx]);
            image.data[y][x] = (RGBColor) {
                    .red = red,
                    .green = green,
                    .blue = blue,
            };
            wordIdx++;
            x++;
        }
        y++;
    }

    fclose(filePtr);
    return image;
}

void readSceneObjects(char*** inputFileWordsByLine, int* line, Scene* scene) {
    // Q: is this the best way to keep track of the allocations?
    int mtlColorAllocationCount = 1;
    int textureAllocationCount = 1;
    int normalAllocationCount = 1;
    int sphereAllocationCount = 1;
    int vertexAllocationCount = 1;
    int vertexNormalAllocationCount = 0;
    int vertexTextureAllocationCount = 0;
    int faceAllocationCount = 1;
    int ellipsoidAllocationCount = 1;

    // todo: break each of these out into a method?
    while (inputFileWordsByLine[*line][0] != NULL) {
        if (strcmp(inputFileWordsByLine[*line][0], "mtlcolor") == 0) {
            if (scene->mtlColorCount >= INITIAL_MTLCOLOR_COUNT * mtlColorAllocationCount) {
                mtlColorAllocationCount++;
                MaterialColor* newMtlColors = (MaterialColor*) realloc(scene->mtlColors, (INITIAL_MTLCOLOR_COUNT * mtlColorAllocationCount) * sizeof(MaterialColor));
                if (newMtlColors == NULL) {
                    fprintf(stderr, "Memory allocation failed for material colors.");
                    exit(-1);
                }
                scene->mtlColors = newMtlColors;
            }
            checkValues(inputFileWordsByLine[*line], 12, "mtlcolor");
            scene->mtlColors[scene->mtlColorCount].diffuseColor = (Vector3) {
                    .x = convertStringToFloat(inputFileWordsByLine[*line][1]),
                    .y = convertStringToFloat(inputFileWordsByLine[*line][2]),
                    .z = convertStringToFloat(inputFileWordsByLine[*line][3]),
            };
            scene->mtlColors[scene->mtlColorCount].specularColor = (Vector3) {
                    .x = convertStringToFloat(inputFileWordsByLine[*line][4]),
                    .y = convertStringToFloat(inputFileWordsByLine[*line][5]),
                    .z = convertStringToFloat(inputFileWordsByLine[*line][6]),
            };
            scene->mtlColors[scene->mtlColorCount].ambientCoefficient = convertStringToFloat(inputFileWordsByLine[*line][7]);
            scene->mtlColors[scene->mtlColorCount].diffuseCoefficient = convertStringToFloat(inputFileWordsByLine[*line][8]);
            scene->mtlColors[scene->mtlColorCount].specularCoefficient = convertStringToFloat(inputFileWordsByLine[*line][9]);
            scene->mtlColors[scene->mtlColorCount].specularExponent = convertStringToFloat(inputFileWordsByLine[*line][10]);
            scene->mtlColors[scene->mtlColorCount].alpha = convertStringToFloat(inputFileWordsByLine[*line][11]);
            scene->mtlColors[scene->mtlColorCount].refractionIndex = convertStringToFloat(inputFileWordsByLine[*line][12]);
            scene->mtlColorCount++;
        } else if (strcmp(inputFileWordsByLine[*line][0], "texture") == 0) {
            if (scene->textureCount >= INITIAL_TEXTURE_COUNT * textureAllocationCount) {
                textureAllocationCount++;
                PPMImage* newTextures = (PPMImage*) realloc(scene->textures, (INITIAL_TEXTURE_COUNT * textureAllocationCount) * sizeof(PPMImage));
                if (newTextures == NULL) {
                    fprintf(stderr, "Memory allocation failed for textures.");
                    exit(-1);
                }
                scene->textures = newTextures;
            }
            checkValues(inputFileWordsByLine[*line], 1, "texture");
            scene->textures[scene->textureCount] = readPPM(inputFileWordsByLine[*line][1]);
            scene->textureCount++;
        } else if (strcmp(inputFileWordsByLine[*line][0], "bump") == 0) {
            if (scene->normalCount >= INITIAL_NORMAL_COUNT * normalAllocationCount) {
                normalAllocationCount++;
                PPMImage* newNormals = (PPMImage*) realloc(scene->normals, (INITIAL_NORMAL_COUNT * normalAllocationCount) * sizeof(PPMImage));
                if (newNormals == NULL) {
                    fprintf(stderr, "Memory allocation failed for normals.");
                    exit(-1);
                }
                scene->normals = newNormals;
            }
            checkValues(inputFileWordsByLine[*line], 1, "bump");
            scene->normals[scene->normalCount] = readPPM(inputFileWordsByLine[*line][1]);
            scene->normalCount++;
        } else if (strcmp(inputFileWordsByLine[*line][0], "sphere") == 0) {
            if (scene->sphereCount >= INITIAL_SPHERE_COUNT * sphereAllocationCount) {
                // todo, bumps apply to other textures below it...
                sphereAllocationCount++;
                Sphere* newSpheres = (Sphere*) realloc(scene->spheres, (INITIAL_SPHERE_COUNT * sphereAllocationCount) * sizeof(Sphere));
                if (newSpheres == NULL) {
                    fprintf(stderr, "Memory allocation failed for spheres.");
                    exit(-1);
                }
                scene->spheres = newSpheres;
            }
            checkValues(inputFileWordsByLine[*line], 4, "sphere");
            Vector3 spherePosition = {
                    .x = convertStringToFloat(inputFileWordsByLine[*line][1]),
                    .y = convertStringToFloat(inputFileWordsByLine[*line][2]),
                    .z = convertStringToFloat(inputFileWordsByLine[*line][3])
            };
            scene->spheres[scene->sphereCount].center = spherePosition;
            scene->spheres[scene->sphereCount].radius = convertStringToFloat(inputFileWordsByLine[*line][4]);
            scene->spheres[scene->sphereCount].mtlColorIdx = scene->mtlColorCount - 1;
            scene->spheres[scene->sphereCount].textureIdx = scene->textureCount - 1;
            scene->spheres[scene->sphereCount].normalIdx = scene->normalCount - 1;
            scene->sphereCount++;
        } else if (strcmp(inputFileWordsByLine[*line][0], "ellipse") == 0) {
            if (scene->ellipsoidCount >= INITIAL_ELLIPSOID_COUNT * ellipsoidAllocationCount) {
                ellipsoidAllocationCount++;
                Ellipsoid* newEllipsoids = (Ellipsoid*) realloc(scene->ellipsoids, (INITIAL_ELLIPSOID_COUNT * ellipsoidAllocationCount) * sizeof(Ellipsoid));
                if (newEllipsoids == NULL) {
                    fprintf(stderr, "Memory allocation failed for ellipsoids.");
                    exit(-1);
                }
                scene->ellipsoids = newEllipsoids;
            }
            checkValues(inputFileWordsByLine[*line], 6, "ellipse");
            Vector3 ellipsoidCenter = {
                    .x = convertStringToFloat(inputFileWordsByLine[*line][1]),
                    .y = convertStringToFloat(inputFileWordsByLine[*line][2]),
                    .z = convertStringToFloat(inputFileWordsByLine[*line][3])
            };
            Vector3 ellipsoidRadius = {
                    .x = convertStringToFloat(inputFileWordsByLine[*line][4]),
                    .y = convertStringToFloat(inputFileWordsByLine[*line][5]),
                    .z = convertStringToFloat(inputFileWordsByLine[*line][6])
            };
            scene->ellipsoids[scene->ellipsoidCount].center = ellipsoidCenter;
            scene->ellipsoids[scene->ellipsoidCount].radius = ellipsoidRadius;
            scene->ellipsoids[scene->ellipsoidCount].mtlColorIdx = scene->mtlColorCount - 1;
            scene->ellipsoids[scene->ellipsoidCount].textureIdx = scene->textureCount - 1;
            scene->ellipsoids[scene->ellipsoidCount].normalIdx = scene->normalCount - 1;
            scene->ellipsoidCount++;
        } else if (strcmp(inputFileWordsByLine[*line][0], "v") == 0) {
            readVertex(inputFileWordsByLine, scene, *line, &vertexAllocationCount);
        } else if (strcmp(inputFileWordsByLine[*line][0], "vn") == 0) {
            readVertexNormal(inputFileWordsByLine, scene, *line, &vertexNormalAllocationCount);
        } else if (strcmp(inputFileWordsByLine[*line][0], "vt") == 0) {
            if (scene->vertexTextureCount == 0) {
                vertexTextureAllocationCount = 1;
                scene->vertexTextures = (TextureCoordinate*) malloc(INITIAL_VERTEX_TEXTURE_COUNT * sizeof(TextureCoordinate));
            } else if (scene->vertexTextureCount >= INITIAL_VERTEX_TEXTURE_COUNT * vertexTextureAllocationCount) {
                vertexTextureAllocationCount++;
                TextureCoordinate* newVertexTextures = (TextureCoordinate*) realloc(scene->vertexTextures, (INITIAL_VERTEX_TEXTURE_COUNT * vertexTextureAllocationCount) * sizeof(TextureCoordinate));
                if (newVertexTextures == NULL) {
                    fprintf(stderr, "Memory allocation failed for vertex textures.");
                    exit(-1);
                }
                scene->vertexTextures = newVertexTextures;
            }
            checkValues(inputFileWordsByLine[*line], 2, "vt");
            scene->vertexTextures[scene->vertexTextureCount] = (TextureCoordinate) {
                    .u = convertStringToFloat(inputFileWordsByLine[*line][1]),
                    .v = convertStringToFloat(inputFileWordsByLine[*line][2]),
            };
            scene->vertexTextureCount++;
        } else if (strcmp(inputFileWordsByLine[*line][0], "f") == 0) {
            if (scene->faceCount >= INITIAL_FACE_COUNT * faceAllocationCount) {
                faceAllocationCount++;
                Face * newFaces = (Face *) realloc(scene->faces, (INITIAL_FACE_COUNT * faceAllocationCount) * sizeof(Face));
                if (newFaces == NULL) {
                    fprintf(stderr, "Memory allocation failed for faces.");
                    exit(-1);
                }
                scene->faces = newFaces;
            }
            checkValues(inputFileWordsByLine[*line], 3, "f");
            scene->faces[scene->faceCount] = (Face) {
                    .v1 = 0,
                    .v2 = 0,
                    .v3 = 0,
                    .vt1 = 0,
                    .vt2 = 0,
                    .vt3 = 0,
                    .vn1 = 0,
                    .vn2 = 0,
                    .vn3 = 0,
                    .mtlColorIdx = scene->mtlColorCount - 1,
                    .textureIdx = scene->textureCount - 1,
                    .normalIdx = scene->normalCount - 1,
            };
            parseFaceValues(inputFileWordsByLine[*line], 1, scene, *line);
            parseFaceValues(inputFileWordsByLine[*line], 2, scene, *line);
            parseFaceValues(inputFileWordsByLine[*line], 3, scene, *line);
            scene->faceCount++;
        }
        (*line)++;
    }
}

void printScene(Scene* scene) {
    printf("--------------------SCENE--------------------\n");
    printf("eye: %f %f %f\n", scene->eye.x, scene->eye.y, scene->eye.z);
    printf("viewdir: %f %f %f\n", scene->viewDir.x, scene->viewDir.y, scene->viewDir.z);
    printf("updir: %f %f %f\n", scene->upDir.x, scene->upDir.y, scene->upDir.z);
    printf("hfov: %f\n", scene->fov.h);
    printf("imsize: %d %d\n", scene->imSize.width, scene->imSize.height);
    printf("bkgcolor: %f %f %f\n", scene->bkgColor.color.x, scene->bkgColor.color.y, scene->bkgColor.color.z);
    printf("parallel: %f\n", scene->parallel.frustumWidth);
    if (scene->lights != NULL) {
        for (int lightIdx = 0; lightIdx < scene->lightCount; lightIdx++) {
            printf("light: %f %f %f %f %f %f %f %f\n", scene->lights[lightIdx].position.x, scene->lights[lightIdx].position.y, scene->lights[lightIdx].position.z, scene->lights[lightIdx].pointOrDirectional, scene->lights[lightIdx].intensity, scene->lights[lightIdx].constantAttenuation, scene->lights[lightIdx].linearAttenuation, scene->lights[lightIdx].quadraticAttenuation);
        }
    }
    if (scene->mtlColors != NULL) {
        for (int mtlColorIdx = 0; mtlColorIdx < scene->mtlColorCount; mtlColorIdx++) {
            printf("mtlcolor: %f %f %f %f %f %f %f %f %f %f\n",
                   scene->mtlColors[mtlColorIdx].diffuseColor.x,
                   scene->mtlColors[mtlColorIdx].diffuseColor.y,
                   scene->mtlColors[mtlColorIdx].diffuseColor.z,
                   scene->mtlColors[mtlColorIdx].specularColor.x,
                   scene->mtlColors[mtlColorIdx].specularColor.y,
                   scene->mtlColors[mtlColorIdx].specularColor.z,
                   scene->mtlColors[mtlColorIdx].ambientCoefficient,
                   scene->mtlColors[mtlColorIdx].diffuseCoefficient,
                   scene->mtlColors[mtlColorIdx].specularCoefficient,
                   scene->mtlColors[mtlColorIdx].specularExponent
            );
        }
    }
    if (scene->textures != NULL) {
        for (int textureIdx = 0; textureIdx < scene->textureCount; textureIdx++) {
            printf("texture: %d %d %d\n", scene->textures[textureIdx].width, scene->textures[textureIdx].height, scene->textures[textureIdx].maxColor);
        }
    }
    if (scene->spheres != NULL) {
        for (int sphereIdx = 0; sphereIdx < scene->sphereCount; sphereIdx++) {
            printf("sphere: %f %f %f %f\n", scene->spheres[sphereIdx].center.x, scene->spheres[sphereIdx].center.y, scene->spheres[sphereIdx].center.z, scene->spheres[sphereIdx].radius);
        }
    }
    if (scene->ellipsoids != NULL) {
        for (int ellipsoidIdx = 0; ellipsoidIdx < scene->ellipsoidCount; ellipsoidIdx++) {
            printf(
                    "ellipse: %f %f %f %f %f %f\n",
                    scene->ellipsoids[ellipsoidIdx].center.x,
                    scene->ellipsoids[ellipsoidIdx].center.y,
                    scene->ellipsoids[ellipsoidIdx].center.z,
                    scene->ellipsoids[ellipsoidIdx].radius.x,
                    scene->ellipsoids[ellipsoidIdx].radius.y,
                    scene->ellipsoids[ellipsoidIdx].radius.z
            );
        }
    }
    if (scene->vertexes != NULL) {
        for (int vertexIdx = 0; vertexIdx < scene->vertexCount; vertexIdx++) {
            printf("vertex: %f %f %f\n", scene->vertexes[vertexIdx].x, scene->vertexes[vertexIdx].y, scene->vertexes[vertexIdx].z);
        }
    }
    if (scene->vertexNormals != NULL) {
        for (int vertexNormalIdx = 0; vertexNormalIdx < scene->vertexNormalCount; vertexNormalIdx++) {
            printf("vertex normal: %f %f %f\n", scene->vertexNormals[vertexNormalIdx].x, scene->vertexNormals[vertexNormalIdx].y, scene->vertexNormals[vertexNormalIdx].z);
        }
    }
    if (scene->vertexTextures != NULL) {
        for (int vertexTextureIdx = 0; vertexTextureIdx < scene->vertexTextureCount; vertexTextureIdx++) {
            printf("vertex texture: %f %f\n", scene->vertexTextures[vertexTextureIdx].u, scene->vertexTextures[vertexTextureIdx].v);
        }
    }
    if (scene->faces != NULL) {
        for (int faceIdx = 0; faceIdx < scene->faceCount; faceIdx++) {
            printf("face: %d %d %d\n", scene->faces[faceIdx].v1, scene->faces[faceIdx].v2, scene->faces[faceIdx].v3);
        }
    }
    printf("---------------------------------------------\n\n");
}

void freeInput(Scene* scene) {
    if (scene->lights != NULL) {
        free(scene->lights);
    }
    if (scene->mtlColors != NULL) {
        free(scene->mtlColors);
    }
    if (scene->textures != NULL) {
        free(scene->textures);
    }
    if (scene->spheres != NULL) {
        free(scene->spheres);
    }
    if (scene->ellipsoids != NULL) {
        free(scene->ellipsoids);
    }
    if (scene->vertexes != NULL) {
        free(scene->vertexes);
    }
    if (scene->vertexNormals != NULL) {
        free(scene->vertexNormals);
    }
    if (scene->faces != NULL) {
        free(scene->faces);
    }
}

#endif