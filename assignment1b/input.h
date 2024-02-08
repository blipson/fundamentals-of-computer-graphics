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

#define MAX_LINE_COUNT 500
#define MAX_WORDS_PER_LINE 50 // This will wrap if they have more than this many words in a line and cause weird behavior
#define MAX_LINE_LENGTH 50
#define KEYWORD_COUNT 11

void checkArgs(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./raytracer1a <path/to/input_file>`");
        exit(-1);
    }
//    if (strcmp(argv[0], "./raytracer1a") != 0) {
//        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./raytracer1a <path/to/input_file>`");
//        exit(-1);
//    }
}

char** readLine(char* line, char** wordsInLine) {
    char* delimiters = " \t\n";
    char* token = strtok(line, delimiters);
    int wordIdx = 0;

    while (token != NULL && wordIdx < MAX_WORDS_PER_LINE) {
        // Remove trailing whitespace
        size_t length = strlen(token);
        while (length > 0 && isspace(token[length - 1])) {
            length--;
        }

        if (length > 0) {
            wordsInLine[wordIdx] = (char *) malloc(strlen(token) + 1);
            if (wordsInLine[wordIdx] == NULL) {
                fprintf(stderr, "Memory allocation error for word in line.");
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
    char* keywords[KEYWORD_COUNT] = {"eye", "viewdir", "updir", "hfov", "imsize", "bkgcolor", "mtlcolor", "sphere", "parallel", "ellipse", "light"};
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        if (strcmp(target, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

char*** readInputFile(char* inputFileName) {
    char*** inputFileWordsByLine = NULL;

    FILE* inputFilePtr;
    inputFilePtr = fopen(inputFileName, "r");

    if (inputFilePtr != NULL) {
        inputFileWordsByLine = malloc(MAX_LINE_COUNT * sizeof(char**));

        if (inputFileWordsByLine == NULL) {
            fprintf(stderr, "Memory allocation error with reading the input file lines.");
            exit(-1);
        }

        char currentLine[MAX_LINE_LENGTH];
        int line = 0;
        while (
                (inputFileWordsByLine[line] = malloc(MAX_WORDS_PER_LINE * sizeof(char*))) != NULL &&
                fgets(currentLine, MAX_LINE_LENGTH, inputFilePtr) != NULL
        ) {
            if (currentLine[0] == '\n' || currentLine[0] == '\0') {
                continue;
            }
            if (line > MAX_LINE_COUNT) {
                fprintf(stderr, "Invalid file format. Too many lines.");
                exit(-1);
            }
            readLine(currentLine, inputFileWordsByLine[line]);
            if (!isKeyword(inputFileWordsByLine[line][0])) {
                fprintf(stderr, "Invalid keyword in input file: %s", inputFileWordsByLine[line][0]);
                exit(-1);
            }
            line++;
        }
    } else {
        fprintf(stderr, "Unable to open the input file specified.");
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
    char *end;
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
        fprintf(stderr, "Improper file format. Invalid integer value: %s", s);
        exit(-1);
    }
    return i;
}

unsigned char convertFloatToUnsignedChar(float normalizedValue) {
    if (normalizedValue < 0.0) {
        normalizedValue = 0.0f;
    } else if (normalizedValue > 1.0) {
        normalizedValue = 1.0f;
    }
    unsigned char result = (unsigned char)(normalizedValue * 255.0);
    return result;
}

void checkValues(char** line, int expectedNumber, char* type) {
    if (line[expectedNumber + 1] != NULL) {
        fprintf(stderr, "Too many values given for '%s', it expects %d.", type, expectedNumber);
        exit(-1);
    }
    for (int i = 1; i <= expectedNumber; i++) {
        if (line[i] == NULL) {
            fprintf(stderr, "Too few values given for '%s', it expects %d.", type, expectedNumber);
            exit(-1);
        }
    }
}

void readSceneSetup(
        char*** inputFileWordsByLine,
        int* line,
        Scene* scene
) {
    while (inputFileWordsByLine[*line][0] != NULL && strcmp(inputFileWordsByLine[*line][0], "mtlcolor") != 0) {
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
        } else if (strcmp(inputFileWordsByLine[*line][0], "imsize") == 0) {
            checkValues(inputFileWordsByLine[*line], 2, "imsize");
            scene->imSize.width = convertStringToInt(inputFileWordsByLine[*line][1]);
            scene->imSize.height = convertStringToInt(inputFileWordsByLine[*line][2]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "bkgcolor") == 0) {
            checkValues(inputFileWordsByLine[*line], 3, "bkgcolor");
            scene->bkgColor.red = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][1]));
            scene->bkgColor.green = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][2]));
            scene->bkgColor.blue = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][3]));
        } else if (strcmp(inputFileWordsByLine[*line][0], "parallel") == 0) {
            checkValues(inputFileWordsByLine[*line], 1, "parallel");
            scene->parallel.frustumWidth = convertStringToFloat(inputFileWordsByLine[*line][1]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "light") == 0) {
            Light* newLights = (Light*) realloc(scene->lights, (scene->lightCount + 1) * sizeof(Light));
            if (newLights == NULL) {
                fprintf(stderr, "Memory allocation failed for light.");
                exit(-1);
            }
            scene->lights = newLights;
            checkValues(inputFileWordsByLine[*line], 5, "light");
            scene->lights[scene->lightCount].position = (Vector3) {
                    .x = convertStringToFloat(inputFileWordsByLine[*line][1]),
                    .y = convertStringToFloat(inputFileWordsByLine[*line][2]),
                    .z = convertStringToFloat(inputFileWordsByLine[*line][3])
            };
            scene->lights[scene->lightCount].w = convertStringToFloat(inputFileWordsByLine[*line][4]);
            scene->lights[scene->lightCount].i = convertStringToFloat(inputFileWordsByLine[*line][5]);
        }
        (*line)++;
    }
}

void readSceneObjects(char*** inputFileWordsByLine, int* line, Scene* scene) {
    while (inputFileWordsByLine[*line][0] != NULL) {
        if (strcmp(inputFileWordsByLine[*line][0], "mtlcolor") == 0) {
            MaterialColor* newMtlColors = (MaterialColor*) realloc(scene->mtlColors, (scene->mtlColorCount + 1) * sizeof(MaterialColor));
            if (newMtlColors == NULL) {
                fprintf(stderr, "Memory allocation failed for material color.");
                exit(-1);
            }
            scene->mtlColors = newMtlColors;
            checkValues(inputFileWordsByLine[*line], 10, "mtlcolor");
            scene->mtlColors[scene->mtlColorCount].diffuseColor = (RGBColor) {
                    .red = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][1])),
                    .green = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][2])),
                    .blue = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][3])),
            };
            scene->mtlColors[scene->mtlColorCount].specularColor = (RGBColor) {
                    .red = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][4])),
                    .green = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][5])),
                    .blue = convertFloatToUnsignedChar(convertStringToFloat(inputFileWordsByLine[*line][6])),
            };
            scene->mtlColors[scene->mtlColorCount].ambientCoefficient = convertStringToFloat(inputFileWordsByLine[*line][7]);
            scene->mtlColors[scene->mtlColorCount].diffuseCoefficient = convertStringToFloat(inputFileWordsByLine[*line][8]);
            scene->mtlColors[scene->mtlColorCount].specularCoefficient = convertStringToFloat(inputFileWordsByLine[*line][9]);
            scene->mtlColors[scene->mtlColorCount].specularExponent = convertStringToFloat(inputFileWordsByLine[*line][10]);

            int objectLine = (*line) + 1;
            while (inputFileWordsByLine[objectLine][0] != NULL &&
                   strcmp(inputFileWordsByLine[objectLine][0], "mtlcolor") != 0) {
                if (strcmp(inputFileWordsByLine[objectLine][0], "sphere") == 0) {
                    Sphere* newSpheres = (Sphere*) realloc(scene->spheres, (scene->sphereCount + 1) * sizeof(Sphere));
                    if (newSpheres == NULL) {
                        fprintf(stderr, "Memory allocation failed for sphere.");
                        exit(-1);
                    }
                    scene->spheres = newSpheres;
                    checkValues(inputFileWordsByLine[objectLine], 4, "sphere");
                    Vector3 spherePosition = {
                            .x = convertStringToFloat(inputFileWordsByLine[objectLine][1]),
                            .y = convertStringToFloat(inputFileWordsByLine[objectLine][2]),
                            .z = convertStringToFloat(inputFileWordsByLine[objectLine][3])
                    };
                    scene->spheres[scene->sphereCount].center = spherePosition;
                    scene->spheres[scene->sphereCount].radius = convertStringToFloat(inputFileWordsByLine[objectLine][4]);
                    scene->spheres[scene->sphereCount].mtlColorIdx = scene->mtlColorCount;
                    scene->sphereCount++;
                } else if (strcmp(inputFileWordsByLine[objectLine][0], "ellipse") == 0) {
                    Ellipse* newEllipses = (Ellipse *) realloc(scene->ellipses, (scene->ellipseCount + 1) * sizeof(Ellipse));
                    if (newEllipses == NULL) {
                        fprintf(stderr, "Memory allocation failed for ellipse.");
                        exit(-1);
                    }
                    scene->ellipses = newEllipses;
                    checkValues(inputFileWordsByLine[objectLine], 6, "ellipse");
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
                    scene->ellipses[scene->ellipseCount].center = ellipseCenter;
                    scene->ellipses[scene->ellipseCount].radius = ellipseRadius;
                    scene->ellipses[scene->ellipseCount].mtlColorIdx = scene->mtlColorCount;
                    scene->ellipseCount++;
                }
                objectLine++;
            }
            scene->mtlColorCount++;
            *line = objectLine;
        } else {
            (*line)++;
        }
    }
}

void printScene(Scene scene) {
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
            printf("mtlcolor: %d %d %d %d %d %d %f %f %f %f\n",
                   scene.mtlColors[mtlColorIdx].diffuseColor.red,
                   scene.mtlColors[mtlColorIdx].diffuseColor.green,
                   scene.mtlColors[mtlColorIdx].diffuseColor.blue,
                   scene.mtlColors[mtlColorIdx].specularColor.red,
                   scene.mtlColors[mtlColorIdx].specularColor.green,
                   scene.mtlColors[mtlColorIdx].specularColor.blue,
                   scene.mtlColors[mtlColorIdx].ambientCoefficient,
                   scene.mtlColors[mtlColorIdx].diffuseCoefficient,
                   scene.mtlColors[mtlColorIdx].specularCoefficient,
                   scene.mtlColors[mtlColorIdx].specularExponent
           );
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