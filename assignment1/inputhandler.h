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

int convertStringToInt(char* s) {
    char* end;
    int i = (int) strtol(s, &end, 10);
    if (strcmp("", end) != 0 && *end != 0) {
        fprintf(stderr, "Improper file format. width and height must both be valid integers.");
        exit(-1);
    }
    return i;
}

void readInputHeader(
        char*** inputFileWordsByLine,
        int* line,
        VectorOrPoint3D* eye,
        VectorOrPoint3D* viewDir,
        VectorOrPoint3D* upDir,
        HorizontalFOV* hfov,
        ImSize* imSize,
        RGBColor* bkgColor,
        Parallel* parallel
) {
    while (inputFileWordsByLine[*line][0] != NULL && strcmp(inputFileWordsByLine[*line][0], "mtlcolor") != 0) {
        // todo: handle if there's too many or two few values in any
        if (strcmp(inputFileWordsByLine[*line][0], "eye") == 0) {
            eye->x = convertStringToInt(inputFileWordsByLine[*line][1]);
            eye->y = convertStringToInt(inputFileWordsByLine[*line][2]);
            eye->z = convertStringToInt(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "viewdir") == 0) {
            viewDir->x = convertStringToInt(inputFileWordsByLine[*line][1]);
            viewDir->y = convertStringToInt(inputFileWordsByLine[*line][2]);
            viewDir->z = convertStringToInt(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "updir") == 0) {
            upDir->x = convertStringToInt(inputFileWordsByLine[*line][1]);
            upDir->y = convertStringToInt(inputFileWordsByLine[*line][2]);
            upDir->z = convertStringToInt(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "hfov") == 0) {
            hfov->h = convertStringToInt(inputFileWordsByLine[*line][1]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "imsize") == 0) {
            imSize->width = convertStringToInt(inputFileWordsByLine[*line][1]);
            imSize->height = convertStringToInt(inputFileWordsByLine[*line][2]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "bkgcolor") == 0) {
            bkgColor->red = convertStringToInt(inputFileWordsByLine[*line][1]);
            bkgColor->green = convertStringToInt(inputFileWordsByLine[*line][2]);
            bkgColor->blue = convertStringToInt(inputFileWordsByLine[*line][3]);
        } else if (strcmp(inputFileWordsByLine[*line][0], "parallel") == 0) {
            parallel->frustumWidth = convertStringToInt(inputFileWordsByLine[*line][1]);
        }
        (*line)++;
    }
}

void readInputObjects(char*** inputFileWordsByLine, int* line, MtlColor** mtlColors, int* mtlColorCount) {
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
                    spheres[sphereCount].x = convertStringToInt(inputFileWordsByLine[objectLine][1]);
                    spheres[sphereCount].y = convertStringToInt(inputFileWordsByLine[objectLine][2]);
                    spheres[sphereCount].z = convertStringToInt(inputFileWordsByLine[objectLine][3]);
                    spheres[sphereCount].r = convertStringToInt(inputFileWordsByLine[objectLine][4]);
                    sphereCount++;
                } else if (strcmp(inputFileWordsByLine[objectLine][0], "ellipse") == 0) {
                    Ellipse* newEllipses = (Ellipse *) realloc(ellipses, (ellipseCount + 1) * sizeof(Ellipse));
                    if (newEllipses == NULL) {
                        fprintf(stderr, "Memory allocation failed for ellipse.");
                        exit(-1);
                    }
                    ellipses = newEllipses;
                    // todo: handle if there's too many or two few values in any
                    ellipses[ellipseCount].cx = convertStringToInt(inputFileWordsByLine[objectLine][1]);
                    ellipses[ellipseCount].cy = convertStringToInt(inputFileWordsByLine[objectLine][2]);
                    ellipses[ellipseCount].cz = convertStringToInt(inputFileWordsByLine[objectLine][3]);
                    ellipses[ellipseCount].rx = convertStringToInt(inputFileWordsByLine[objectLine][4]);
                    ellipses[ellipseCount].ry = convertStringToInt(inputFileWordsByLine[objectLine][5]);
                    ellipses[ellipseCount].rz = convertStringToInt(inputFileWordsByLine[objectLine][6]);
                    ellipseCount++;
                }
                objectLine++;
            }
            MtlColor* newMtlColors = (MtlColor *) realloc(*mtlColors, ((*mtlColorCount) + 1) * sizeof(MtlColor));
            if (newMtlColors == NULL) {
                fprintf(stderr, "Memory allocation failed for material color.");
                exit(-1);
            }
            *mtlColors = newMtlColors;

            RGBColor color;
            // todo: handle if there's too many or two few values in any
            color.red = convertStringToInt(inputFileWordsByLine[*line][1]);
            color.green = convertStringToInt(inputFileWordsByLine[*line][2]);
            color.blue = convertStringToInt(inputFileWordsByLine[*line][3]);


            (*mtlColors)[*mtlColorCount].spheres = spheres;
            (*mtlColors)[*mtlColorCount].sphereCount = sphereCount;
            (*mtlColors)[*mtlColorCount].ellipses = ellipses;
            (*mtlColors)[*mtlColorCount].ellipseCount = ellipseCount;
            (*mtlColors)[*mtlColorCount].color = color;
            (*mtlColorCount)++;
            *line = objectLine;
        } else {
            (*line)++;
        }
    }
}

void printInput(
        VectorOrPoint3D eye,
        VectorOrPoint3D viewDir,
        VectorOrPoint3D upDir,
        HorizontalFOV hfov,
        ImSize imSize,
        RGBColor bkgColor,
        Parallel parallel,
        MtlColor* mtlColors,
        int mtlColorCount
) {
    printf("eye: %d %d %d\n", eye.x, eye.y, eye.z);
    printf("viewdir: %d %d %d\n", viewDir.x, viewDir.y, viewDir.z);
    printf("updir: %d %d %d\n", upDir.x, upDir.y, upDir.z);
    printf("hfov: %d\n", hfov.h);
    printf("imsize: %d %d\n", imSize.width, imSize.height);
    printf("bkgcolor: %d %d %d\n", bkgColor.red, bkgColor.green, bkgColor.blue);
    printf("parallel: %d\n", parallel.frustumWidth);
    if (mtlColors != NULL) {
        for (int mtlColorIdx = 0; mtlColorIdx < mtlColorCount; mtlColorIdx++) {
            printf("mtlcolor: %d %d %d\n", mtlColors[mtlColorIdx].color.red, mtlColors[mtlColorIdx].color.green, mtlColors[mtlColorIdx].color.blue);
            for (int sphereIdx = 0; sphereIdx < mtlColors[mtlColorIdx].sphereCount; sphereIdx++) {
                printf("sphere: %d %d %d\n", mtlColors[mtlColorIdx].spheres[sphereIdx].x, mtlColors[mtlColorIdx].spheres[sphereIdx].y, mtlColors[mtlColorIdx].spheres[sphereIdx].z);
            }
            for (int ellipseIdx = 0; ellipseIdx < mtlColors[mtlColorIdx].ellipseCount; ellipseIdx++) {
                printf(
                        "ellipse: %d %d %d %d %d %d\n",
                        mtlColors[mtlColorIdx].ellipses[ellipseIdx].cx,
                        mtlColors[mtlColorIdx].ellipses[ellipseIdx].cy,
                        mtlColors[mtlColorIdx].ellipses[ellipseIdx].cz,
                        mtlColors[mtlColorIdx].ellipses[ellipseIdx].rx,
                        mtlColors[mtlColorIdx].ellipses[ellipseIdx].ry,
                        mtlColors[mtlColorIdx].ellipses[ellipseIdx].rz
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