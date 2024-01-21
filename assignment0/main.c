#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <math.h>

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGBColor;

static int maxInputFileNameLength = 100;
/*
* 17 is the minimum number of chars required to allow for an input.txt file
* with the maximum display resolution of 7680x4320 with this format:
                        imsize 7680 4320
*/
static int maxInputFileSize = 17;
static char* imSizeKeyword = "imsize";
static char* outputFileSuffix = "_out.ppm";
static char* magicNumber = "P3";
static char* maxColorComponentValue = "255";
/*
* 5 is chosen to ensure a max line length of 70 character. If each pixel
* is the maximum character count "255 255 255" with "\t" in between,
* it makes 12 char per pixel, meaning 5 pixels per line to stay below 70.
*/
static int maxPixelsOnLine = 5;

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

void checkArgs(int argc, char* argv[]) {
    if (argc > 3 || argc < 2) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./assignment0 <path/to/input_file.txt> <pattern>`");
        exit(-1);
    } else if (argc > 2) {
        if (strcmp(argv[2], "solid") != 0 && strcmp(argv[2], "grid") != 0 && strcmp(argv[2], "mandelbrot") != 0 &&
                strcmp(argv[2], "julia") != 0 && strcmp(argv[2], "checkerboard") != 0 && strcmp(argv[2], "gaussian") != 0) {
            fprintf(stderr, "Incorrect usage. Pattern options are [solid, grid, mandelbrot, julia, checkerboard, gaussian].");
            exit(-1);
        }
    }

    if (strlen(argv[1]) > maxInputFileNameLength) {
        fprintf(stderr, "Input file name specified is too long. It must be under %d characters.", maxInputFileNameLength);
        exit(-1);
    }

    if (strcmp(substr(argv[1], (int) (strlen(argv[1]) - 4), (int) (strlen(argv[1]))), ".txt") != 0) {
        fprintf(stderr, "Incorrect input file format. Input file must be a '.txt' file.");
        exit(-1);
    }
}

void getInputFileContent(char* inputFileName, char* inputFileContents) {
    FILE* inputFilePtr;
    inputFilePtr = fopen(inputFileName, "r");

    if (inputFilePtr != NULL) {
        struct stat inputFileStatus;
        stat(inputFileName, &inputFileStatus);

        if (inputFileStatus.st_size > maxInputFileSize) {
            fprintf(stderr, "Input file is too big. It must be under %d characters.", maxInputFileSize);
            exit(-1);
        }

        fgets(inputFileContents, maxInputFileSize, inputFilePtr);

        char extraLines[maxInputFileSize];
        fgets(extraLines, maxInputFileSize, inputFilePtr);
        if (strlen(extraLines) > 0) {
            fprintf(stderr, "Improper file format. No extra lines should be included beyond 'imsize <width> <height>'");
            exit(-1);
        }
    } else {
        fprintf(stderr, "Unable to open the input file specified.");
        exit(-1);
    }
}

bool isWhiteSpace(char c) {
    return c == '\n' || c == '\t' || c == ' ';
}

void readFileContent(char* inputFileContents, char** fileContent) {
    int i = 0;
    while (!isWhiteSpace(inputFileContents[i])) {
        i++;
    }
    fileContent[0] = substr(inputFileContents, 0, i);

    // skip whitespace character.
    i++;

    int widthStart = i;
    while (!isWhiteSpace(inputFileContents[i])) {
        i++;
    }
    fileContent[1] = substr(inputFileContents, widthStart, i);

    // skip whitespace character.
    i++;

    int heightStart = i;
    while (!isWhiteSpace(inputFileContents[i])) {
        i++;
    }
    fileContent[2] = substr(inputFileContents, heightStart, i);

    int extraStart = i;
    while (inputFileContents[i] != '\0') {
        i++;
    }
    char* extraInput = substr(inputFileContents, extraStart, i);
    if (strlen(extraInput) > 1) {
        fprintf(stderr, "Improper file format. No extra info should be included beyond 'imsize <width> <height>'.");
        exit(-1);
    }
}

void checkFileContent(char** fileContent) {
    if (strcmp(fileContent[0], imSizeKeyword) != 0) {
        fprintf(stderr, "Improper file format. The keyword 'imsize' must come first.");
        exit(-1);
    }

    if (strlen(fileContent[1]) > 4 || strlen(fileContent[1]) <= 0) {
        fprintf(stderr, "Improper file format. width must be present and be max of 4 characters long.");
        exit(-1);
    }

    if (strlen(fileContent[2]) > 4 || strlen(fileContent[2]) <= 0) {
        fprintf(stderr, "Improper file format. height must be present and be max of 4 characters long.");
        exit(-1);
    }
}

int convertStringToPositiveInt(char* s) {
    char* end;
    int i = (int) strtol(s, &end, 10);
    if (strcmp("", end) != 0 && *end != 0) {
        fprintf(stderr, "Improper file format. width and height must both be valid integers.");
        exit(-1);
    }
    if (i < 0) {
        fprintf(stderr, "Improper file format. width and height must both be positive integers.");
        exit(-1);
    }
    return i;
}

FILE* openOutputFile(char* inputFileName, char* optionalPattern) {
    char outputFileName[maxInputFileNameLength + 9];
    snprintf(outputFileName, sizeof(outputFileName), "%s_%s%s", substr(inputFileName, 0, strlen(inputFileName) - 4), optionalPattern, outputFileSuffix);

    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");
    return outputFilePtr;
}

void writeHeader(FILE* outputFilePtr, int width, int height) {
    fprintf(outputFilePtr, "%s\n%d %d\n%s\n", magicNumber, width, height, maxColorComponentValue);
}

void enforceMaxPixelsOnLine(int x, FILE* outputFilePtr) {
    if (x % (maxPixelsOnLine) == (maxPixelsOnLine - 1)) {
        fprintf(outputFilePtr, "\n");
    } else {
        fprintf(outputFilePtr, "\t");
    }
}

void writeSolidColorContents(FILE* outputFilePtr, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fprintf(outputFilePtr, "255 222 111");
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}

void writeGridContents(FILE* outputFilePtr, int width, int height) {
    bool firstColor = true;
    int squareSize = width / 10;
    bool dividesWithoutRemainder = width % 10 == 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if ((x % squareSize == 0) && (dividesWithoutRemainder || x != 0)) {
                firstColor = !firstColor;
            }
            int color = firstColor ? 255 : 0;
            fprintf(outputFilePtr, "%d %d %d", color, color, color);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
        if (y % squareSize == 0) {
            firstColor = !firstColor;
        }
    }
}

RGBColor mandelbrot(int x, int y, int width, int height, int maxIter) {
    double real = (x - width / 2.0) * 4.0 / width;
    double imaginary = (y - height / 2.0) * 4.0 / height;

    double realTemp, imagTemp;
    double realSquared, imagSquared;

    int i = 0;

    realTemp = real;
    imagTemp = imaginary;

    while (i < maxIter) {
        realSquared = real * real - imaginary * imaginary;
        imagSquared = 2.0 * real * imaginary;

        real = realSquared + realTemp;
        imaginary = imagSquared + imagTemp;

        if (real * real + imaginary * imaginary > 16.0) {
            break;
        }

        i++;
    }

    double squaredMagnitudeLog = log(real * real + imaginary * imaginary) / 2.0;
    double smoothIterationCount = log(squaredMagnitudeLog / log(2.0)) / log(2.0);
    i = (int)(i + 1 - smoothIterationCount);

    RGBColor color;
    color.red = (i * 5) % 255;
    color.green = (i * 10) % 255;
    color.blue = (i * 20) % 255;

    if (i >= maxIter) {
        RGBColor black;
        black.red = 0;
        black.green = 0;
        black.blue = 0;
        return black;
    }

    return color;
}

void writeMandelbrotContents(FILE* outputFilePtr, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor pixelColor = mandelbrot(x, y, width, height, 100000);
            fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
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

void writeJuliaContents(FILE* outputFilePtr, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor pixelColor = julia(x, y, width, height, 1000000, -0.7, 0.27015);
            fprintf(outputFilePtr, "%d %d %d", pixelColor.red, pixelColor.green, pixelColor.blue);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}

int pointInsideCircle(int x, int y, int centerX, int centerY, int radius) {
    int distance = (x - centerX) * (x - centerX) + (y - centerY) * (y - centerY);
    return distance <= radius * radius;
}

void writeCheckerboardContents(FILE* outputFilePtr, int width, int height) {
    int squareSize = width / 10;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool alternatingColor = ((x / squareSize) % 2 == 0) ^ ((y / squareSize) % 2 == 0);
            RGBColor color;
            int gridColor = alternatingColor ? 255 : 0;
            color.red = gridColor;
            color.green = 0;
            color.blue = 0;
            int circleX = x % squareSize;
            int circleY = y % squareSize;
            if (pointInsideCircle(circleX, circleY, squareSize / 2, squareSize / 2, squareSize / 3)) {
                color.red = alternatingColor ? 0 : 255;
                color.green = 0;
                color.blue = 0;
            }
            fprintf(outputFilePtr, "%d %d %d", color.red, color.green, color.blue);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}

double generateGaussianNoise() {
    double u1 = ((double)rand() / RAND_MAX);
    double u2 = ((double)rand() / RAND_MAX);
    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return (z + 3.0) / 6.0;
}

void writeGaussianNoiseContents(FILE* outputFilePtr, int width, int height) {
    double intensity = 30.0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double noise = generateGaussianNoise() * intensity;
            int color = (int)(noise * 255);
            fprintf(outputFilePtr, "%d %d %d", color, color, color);
            enforceMaxPixelsOnLine(x, outputFilePtr);
        }
    }
}


int main(int argc, char* argv[]) {
    checkArgs(argc, argv);

    char* optionalPattern;
    if (argc == 3) {
        optionalPattern = argv[2];
    }

    char inputFileContent[maxInputFileSize];
    getInputFileContent(argv[1], inputFileContent);

    char* fileContent[3];
    readFileContent(inputFileContent, fileContent);
    checkFileContent(fileContent);

    int width = convertStringToPositiveInt(fileContent[1]);
    int height = convertStringToPositiveInt(fileContent[2]);

    FILE* outputFilePtr = openOutputFile(argv[1], argv[2]);
    writeHeader(outputFilePtr, width, height);
    if (argc == 2 || strcmp(optionalPattern, "solid") == 0) {
        writeSolidColorContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "grid") == 0) {
        writeGridContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "mandelbrot") == 0) {
        writeMandelbrotContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "julia") == 0) {
        writeJuliaContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "checkerboard") == 0) {
        writeCheckerboardContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "gaussian") == 0) {
        writeGaussianNoiseContents(outputFilePtr, width, height);
    }

    fclose(outputFilePtr);
    return 0;
}
