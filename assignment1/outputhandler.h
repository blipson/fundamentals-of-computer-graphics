#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_OUTPUTHANDLER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_OUTPUTHANDLER_H

#define MAX_INPUT_FILE_NAME_LENGTH 100
#define OUTPUT_FILE_SUFFIX ".ppm"
#define MAGIC_NUMBER "P3"
#define MAX_COLOR_COMPONENT_VALUE "255"
#define MAX_PIXELS_ON_LINE 5

FILE* openOutputFile(char* inputFileName) {
    char outputFileName[MAX_INPUT_FILE_NAME_LENGTH + 3];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", inputFileName, OUTPUT_FILE_SUFFIX);

    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");
    return outputFilePtr;
}

void writeHeader(FILE* outputFilePtr, int width, int height) {
    fprintf(outputFilePtr, "%s\n%d %d\n%s\n", MAGIC_NUMBER, width, height, MAX_COLOR_COMPONENT_VALUE);
}


RGBColor julia(int x, int y, int width, int height, int maxIter, double realConstant, double imaginaryConstant) {
    double zx = 1.5 * (x - width / 2.0) / (0.5 * width);
    double zy = (y - height / 2.0) / (0.5 * height);

    double realSquared, imagSquared;

    int i = 0;

    while (i < maxIter) {
        realSquared = zx * zx - zy * zy;
        imagSquared = 2.0 * zx * zy;

        zx = realSquared + realConstant;
        zy = imagSquared + imaginaryConstant;

        if (realSquared + imagSquared > 16.0) {
            break;
        }

        i++;
    }

    RGBColor color;
    color.red = (i * 5) % 255;
    color.green = (i * 10) % 255;
    color.blue = (i * 20) % 255;

    return color;
}

void enforceMaxPixelsOnLine(int x, FILE* outputFilePtr) {
    if (x % (MAX_PIXELS_ON_LINE) == (MAX_PIXELS_ON_LINE - 1)) {
        fprintf(outputFilePtr, "\n");
    } else {
        fprintf(outputFilePtr, "\t");
    }
}

void writeJuliaContents(FILE* outputFilePtr, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor pixelColor = julia(x, y, width, height, 10000, -0.7, 0.27015);
            fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}

#endif
