#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>

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
    if (argc > 3) {
        fprintf(stderr, "Incorrect usage. Correct usage is `$ ./assignment0 <path/to/input_file.txt> <pattern>`");
        exit(-1);
    } else if (argc > 2) {
        if (strcmp(argv[2], "solid") != 0 && strcmp(argv[2], "grid") != 0) {
            fprintf(stderr, "Incorrect usage. Pattern options are [solid, grid].");
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
        fprintf(stderr, "Improper file format. Width must be present and be max of 4 characters long.");
        exit(-1);
    }

    if (strlen(fileContent[2]) > 4 || strlen(fileContent[2]) <= 0) {
        fprintf(stderr, "Improper file format. Height must be present and be max of 4 characters long.");
        exit(-1);
    }
}

int convertStringToPositiveInt(char* s) {
    char* end;
    int i = (int) strtol(s, &end, 10);
    if (strcmp("", end) != 0 && *end != 0) {
        fprintf(stderr, "Improper file format. Width and height must both be valid integers.");
        exit(-1);
    }
    if (i < 0) {
        fprintf(stderr, "Improper file format. Width and height must both be positive integers.");
        exit(-1);
    }
    return i;
}

FILE* openOutputFile(char* inputFileName) {
    char outputFileName[maxInputFileNameLength + 9];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", substr(inputFileName, 0, strlen(inputFileName) - 4), outputFileSuffix);

    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");
    return outputFilePtr;
}

void writeHeader(FILE* outputFilePtr, int width, int height) {
    fprintf(outputFilePtr, "%s\n%d %d\n%s\n", magicNumber, width, height, maxColorComponentValue);
}

void enforceMaxPixelsOnLine(int w, FILE* outputFilePtr) {
    if (w % (maxPixelsOnLine) == (maxPixelsOnLine - 1)) {
        fprintf(outputFilePtr, "\n");
    } else {
        fprintf(outputFilePtr, "\t");
    }
}

void writeSolidColorContents(FILE* outputFilePtr, int width, int height) {
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            fprintf(outputFilePtr, "255 222 111");
            enforceMaxPixelsOnLine(w, outputFilePtr);
        }
    }
}

void writeGridContents(FILE* outputFilePtr, int width, int height) {
    bool firstColor = true;
    int squareSize = width/10;
    bool dividesWithoutRemainder = width%10 == 0;
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            if ((w % squareSize == 0) && (dividesWithoutRemainder || w != 0)) {
                firstColor = !firstColor;
            }
            if (firstColor) {
                fprintf(outputFilePtr, "255 222 111");
            } else {
                fprintf(outputFilePtr, "0 0 200");
            }
            enforceMaxPixelsOnLine(w, outputFilePtr);
        }
        if (h % squareSize == 0) {
            firstColor = !firstColor;
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

    FILE* outputFilePtr = openOutputFile(argv[1]);
    writeHeader(outputFilePtr, width, height);
    if (argc == 2 || strcmp(optionalPattern, "solid") == 0) {
        writeSolidColorContents(outputFilePtr, width, height);
    } else if (strcmp(optionalPattern, "grid") == 0) {
        writeGridContents(outputFilePtr, width, height);
    }

    fclose(outputFilePtr);
    return 0;
}
