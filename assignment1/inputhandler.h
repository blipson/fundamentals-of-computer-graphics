#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUTHANDLER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUTHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <_ctype.h>
#include "types.h"

#define MAX_LINE_COUNT 500
#define MAX_WORDS_PER_LINE 8 // This will wrap if they have more than this many words in a line and cause weird behavior
#define MAX_LINE_LENGTH 33
#define KEYWORD_COUNT 10

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

char* substr(char* s, int x, int y) {
    char* ret = malloc(strlen(s) + 1);
    char* p = ret;
    char* q = &s[x];

    if (ret == NULL) {
        fprintf(stderr, "Can not copy string.");
        exit(-1);
    } else {
        while (x < y) {
            *p++ = *q++;
            x++;
        }
        *p = '\0';
    }
    return ret;
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
    char* keywords[KEYWORD_COUNT] = {"eye", "viewdir", "updir", "hfov", "imsize", "bkgcolor", "mtlcolor", "sphere", "parallel", "ellipse"};
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

double convertStringToDouble(char* s) {
    char *end; // To check for conversion errors
    double result = strtod(s, &end);
    if (*end == '\0') {
        return result;
    } else {
        fprintf(stderr, "Conversion failed. Invalid double value: %s", s);
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

unsigned char convertDoubleToUnsignedChar(double normalizedValue) {
    if (normalizedValue < 0.0) {
        normalizedValue = 0.0;
    } else if (normalizedValue > 1.0) {
        normalizedValue = 1.0;
    }
    unsigned char result = (unsigned char)(normalizedValue * 255.0);
    return result;
}

void readInputScene(
        char*** inputFileWordsByLine,
        int* line,
        Scene* scene
) {
    while (inputFileWordsByLine[*line][0] != NULL && strcmp(inputFileWordsByLine[*line][0], "mtlcolor") != 0) {
        // todo: handle if there's too many or two few values in any
        if (strcmp(inputFileWordsByLine[*line][0], "eye") == 0) {
            scene->eye.x = convertStringToDouble(inputFileWordsByLine[*line][1]);
            scene->eye.y = convertStringToDouble(inputFileWordsByLine[*line][2]);
            scene->eye.z = convertStringToDouble(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "viewdir") == 0) {
            scene->viewDir.x = convertStringToDouble(inputFileWordsByLine[*line][1]);
            scene->viewDir.y = convertStringToDouble(inputFileWordsByLine[*line][2]);
            scene->viewDir.z = convertStringToDouble(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "updir") == 0) {
            scene->upDir.x = convertStringToDouble(inputFileWordsByLine[*line][1]);
            scene->upDir.y = convertStringToDouble(inputFileWordsByLine[*line][2]);
            scene->upDir.z = convertStringToDouble(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "hfov") == 0) {
            scene->fov.h = convertStringToDouble(inputFileWordsByLine[*line][1]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "imsize") == 0) {
            scene->imSize.width = convertStringToInt(inputFileWordsByLine[*line][1]);
            scene->imSize.height = convertStringToInt(inputFileWordsByLine[*line][2]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "bkgcolor") == 0) {
            scene->bkgColor.red = convertDoubleToUnsignedChar(convertStringToDouble(inputFileWordsByLine[*line][1]));
            scene->bkgColor.green = convertDoubleToUnsignedChar(convertStringToDouble(inputFileWordsByLine[*line][2]));
            scene->bkgColor.blue = convertDoubleToUnsignedChar(convertStringToDouble(inputFileWordsByLine[*line][3]));
        } else if (strcmp(inputFileWordsByLine[*line][0], "parallel") == 0) {
            scene->parallel.frustumWidth = convertStringToDouble(inputFileWordsByLine[*line][1]);
        }
        (*line)++;
    }
}

void readInputObjects(char*** inputFileWordsByLine, int* line, Scene* scene) {
    while (inputFileWordsByLine[*line][0] != NULL) {
        if (strcmp(inputFileWordsByLine[*line][0], "mtlcolor") == 0) {
            Sphere* spheres = NULL;
            Ellipse* ellipses = NULL;
            int objectLine = (*line) + 1;
            int sphereCount = 0;
            int ellipseCount = 0;
            while (inputFileWordsByLine[objectLine][0] != NULL &&
                   strcmp(inputFileWordsByLine[objectLine][0], "mtlcolor") != 0) {
                if (strcmp(inputFileWordsByLine[objectLine][0], "sphere") == 0) {
                    Sphere* newSpheres = (Sphere *) realloc(spheres, (sphereCount + 1) * sizeof(Sphere));
                    if (newSpheres == NULL) {
                        fprintf(stderr, "Memory allocation failed for sphere.");
                        exit(-1);
                    }
                    spheres = newSpheres;
                    // todo: handle if there's too many or two few values in any
                    Vector3 spherePosition = {
                            .x = convertStringToDouble(inputFileWordsByLine[objectLine][1]),
                            .y = convertStringToDouble(inputFileWordsByLine[objectLine][2]),
                            .z = convertStringToDouble(inputFileWordsByLine[objectLine][3])
                    };
                    spheres[sphereCount].center = spherePosition;
                    spheres[sphereCount].radius = convertStringToDouble(inputFileWordsByLine[objectLine][4]);
                    sphereCount++;
                } else if (strcmp(inputFileWordsByLine[objectLine][0], "ellipse") == 0) {
                    Ellipse* newEllipses = (Ellipse *) realloc(ellipses, (ellipseCount + 1) * sizeof(Ellipse));
                    if (newEllipses == NULL) {
                        fprintf(stderr, "Memory allocation failed for ellipse.");
                        exit(-1);
                    }
                    ellipses = newEllipses;
                    Vector3 ellipsePosition = {
                            .x = convertStringToDouble(inputFileWordsByLine[objectLine][1]),
                            .y = convertStringToDouble(inputFileWordsByLine[objectLine][2]),
                            .z = convertStringToDouble(inputFileWordsByLine[objectLine][3])
                    };
                    Vector3 ellipseRadius = {
                            .x = convertStringToDouble(inputFileWordsByLine[objectLine][4]),
                            .y = convertStringToDouble(inputFileWordsByLine[objectLine][5]),
                            .z = convertStringToDouble(inputFileWordsByLine[objectLine][6])
                    };
                    // todo: handle if there's too many or two few values in any
                    ellipses[ellipseCount].center.x = convertStringToDouble(inputFileWordsByLine[objectLine][1]);
                    ellipses[ellipseCount].center.y = convertStringToDouble(inputFileWordsByLine[objectLine][2]);
                    ellipses[ellipseCount].center.z = convertStringToDouble(inputFileWordsByLine[objectLine][3]);
                    ellipses[ellipseCount].radius.x = convertStringToDouble(inputFileWordsByLine[objectLine][4]);
                    ellipses[ellipseCount].radius.y = convertStringToDouble(inputFileWordsByLine[objectLine][5]);
                    ellipses[ellipseCount].radius.z = convertStringToDouble(inputFileWordsByLine[objectLine][6]);
                    ellipseCount++;
                }
                objectLine++;
            }
            MtlColor* newMtlColors = (MtlColor *) realloc((*scene).mtlColors, ((*scene).mtlColorCount + 1) * sizeof(MtlColor));
            if (newMtlColors == NULL) {
                fprintf(stderr, "Memory allocation failed for material color.");
                exit(-1);
            }
            (*scene).mtlColors = newMtlColors;

            RGBColor color;
            // todo: handle if there's too many or two few values in any
            color.red = convertDoubleToUnsignedChar(convertStringToDouble(inputFileWordsByLine[*line][1]));
            color.green = convertDoubleToUnsignedChar(convertStringToDouble(inputFileWordsByLine[*line][2]));
            color.blue = convertDoubleToUnsignedChar(convertStringToDouble(inputFileWordsByLine[*line][3]));


            (*scene).mtlColors[(*scene).mtlColorCount].spheres = spheres;
            (*scene).mtlColors[(*scene).mtlColorCount].sphereCount = sphereCount;
            (*scene).mtlColors[(*scene).mtlColorCount].ellipses = ellipses;
            (*scene).mtlColors[(*scene).mtlColorCount].ellipseCount = ellipseCount;
            (*scene).mtlColors[(*scene).mtlColorCount].color = color;
            (*scene).mtlColorCount++;
            *line = objectLine;
        } else {
            (*line)++;
        }
    }
}

void printInput(Scene scene) {
    printf("eye: %lf %lf %lf\n", scene.eye.x, scene.eye.y, scene.eye.z);
    printf("viewdir: %lf %lf %lf\n", scene.viewDir.x, scene.viewDir.y, scene.viewDir.z);
    printf("updir: %lf %lf %lf\n", scene.upDir.x, scene.upDir.y, scene.upDir.z);
    printf("hfov: %lf\n", scene.fov.h);
    printf("imsize: %d %d\n", scene.imSize.width, scene.imSize.height);
    printf("bkgcolor: %d %d %d\n", scene.bkgColor.red, scene.bkgColor.green, scene.bkgColor.blue);
    printf("parallel: %lf\n", scene.parallel.frustumWidth);
    if (scene.mtlColors != NULL) {
        for (int mtlColorIdx = 0; mtlColorIdx < scene.mtlColorCount; mtlColorIdx++) {
            printf("mtlcolor: %d %d %d\n", scene.mtlColors[mtlColorIdx].color.red, scene.mtlColors[mtlColorIdx].color.green, scene.mtlColors[mtlColorIdx].color.blue);
            for (int sphereIdx = 0; sphereIdx < scene.mtlColors[mtlColorIdx].sphereCount; sphereIdx++) {
                printf("sphere: %lf %lf %lf %lf\n", scene.mtlColors[mtlColorIdx].spheres[sphereIdx].center.x, scene.mtlColors[mtlColorIdx].spheres[sphereIdx].center.y, scene.mtlColors[mtlColorIdx].spheres[sphereIdx].center.z, scene.mtlColors[mtlColorIdx].spheres[sphereIdx].radius);
            }
            for (int ellipseIdx = 0; ellipseIdx < scene.mtlColors[mtlColorIdx].ellipseCount; ellipseIdx++) {
                printf(
                        "ellipse: %lf %lf %lf %lf %lf %lf\n",
                        scene.mtlColors[mtlColorIdx].ellipses[ellipseIdx].center.x,
                        scene.mtlColors[mtlColorIdx].ellipses[ellipseIdx].center.y,
                        scene.mtlColors[mtlColorIdx].ellipses[ellipseIdx].center.z,
                        scene.mtlColors[mtlColorIdx].ellipses[ellipseIdx].radius.x,
                        scene.mtlColors[mtlColorIdx].ellipses[ellipseIdx].radius.y,
                        scene.mtlColors[mtlColorIdx].ellipses[ellipseIdx].radius.z
                );
            }
        }
    }
}

void freeInput(MtlColor* mtlColors, int mtlColorCount) {
    if (mtlColors != NULL) {
        for (int i = 0; i < mtlColorCount; i++) {
            free(mtlColors[i].spheres);
            free(mtlColors[i].ellipses);
        }
        free(mtlColors);
    }
}

#endif