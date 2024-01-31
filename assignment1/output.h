#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_OUTPUT_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_OUTPUT_H

#include "render.h"
#include "stringhelper.h"

#define MAX_INPUT_FILE_NAME_LENGTH 100
#define OUTPUT_FILE_SUFFIX ".ppm"
#define MAGIC_NUMBER "P3"
#define MAX_COLOR_COMPONENT_VALUE "255"
#define MAX_PIXELS_ON_LINE 5

FILE* openOutputFile(char* inputFileName) {
    if (!endsWith(inputFileName, ".txt")) {
        fprintf(stderr, "Incorrect input file format. Input file must be a '.txt' file.");
        exit(-1);
    }
    char outputFileName[MAX_INPUT_FILE_NAME_LENGTH + 3];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", substr(inputFileName, 0, strlen(inputFileName) - 4), OUTPUT_FILE_SUFFIX);

    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");
    return outputFilePtr;
}

void writeHeader(FILE* outputFilePtr, int width, int height) {
    fprintf(outputFilePtr, "%s\n%d %d\n%s\n", MAGIC_NUMBER, width, height, MAX_COLOR_COMPONENT_VALUE);
}

void enforceMaxPixelsOnLine(FILE* outputFilePtr, int x, int width) {
    if ((x % (MAX_PIXELS_ON_LINE) == (MAX_PIXELS_ON_LINE - 1)) || x == width - 1) {
        fprintf(outputFilePtr, "\n");
    } else {
        fprintf(outputFilePtr, "\t");
    }
}

void writePixel(FILE* outputFilePtr, RGBColor pixelColor, int x, int width) {
    fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
    enforceMaxPixelsOnLine(outputFilePtr, x, width);
}

#endif
