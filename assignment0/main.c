#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

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

int main(int argc, char* argv[]) {
    static int maxInputFileNameLength = 100;
    /*
     * 17 is the minimum number of chars required to allow for an input.txt file
     * with the maximum display resolution of 7680x4320 with this format:
                        imsize 7680 4320
    */
    static int maxInputFileSize = 17;
    static char* outputFileSuffix = "_out.ppm";
    static char* imSizeKeyword = "imsize";
    /*
     * 5 is chosen to ensure a max line length of 70 character. If each pixel
     * is the maximum character count "255 255 255" with "\t" in between,
     * it makes 12 char per pixel, meaning 5 pixels per line to stay below 70.
     */
    static int maxPixelsOnLine = 5;

    char* inputFileName = argv[1];

    if (strlen(inputFileName) > maxInputFileNameLength) {
        fprintf(stderr, "Input file name specified is too long. It must be under %d characters.", maxInputFileNameLength);
        exit(-1);
    }


    FILE* inputFilePtr;
    inputFilePtr = fopen(inputFileName, "r");
    char inputFileContents[maxInputFileSize];

    if (inputFilePtr != NULL) {
        struct stat inputFileStatus;
        stat(inputFileName, &inputFileStatus);

        if (inputFileStatus.st_size > maxInputFileSize) {
            fprintf(stderr, "Input file is too big. It must be under %d characters.", maxInputFileSize);
            exit(-1);
        }

        fgets(inputFileContents, maxInputFileSize, inputFilePtr);
    } else {
        fprintf(stderr, "Unable to open the input.txt file specified.");
        exit(-1);
    }

    int i = 0;
    while (inputFileContents[i] != '\n' && inputFileContents[i] != '\t' && inputFileContents[i] != ' ') {
        i++;
    }
    char* inputImSizeKeyword = substr(inputFileContents, 0, i);

    // skip whitespace character.
    i++;

    int widthStart = i;
    while (inputFileContents[i] != '\n' && inputFileContents[i] != '\t' && inputFileContents[i] != ' ') {
        i++;
    }
    char* inputWidth = substr(inputFileContents, widthStart, i);

    // skip whitespace character.
    i++;

    int heightStart = i;
    while (inputFileContents[i] != '\n' && inputFileContents[i] != '\t' && inputFileContents[i] != ' ') {
        i++;
    }
    char* inputHeight = substr(inputFileContents, heightStart, i);

    if (strcmp(inputImSizeKeyword, "imsize") != 0) {
        fprintf(stderr, "Improper input.txt file format. The keyword 'imsize' must come first.");
        exit(-1);
    }

    if (strlen(inputWidth) > 4) {
        fprintf(stderr, "Improper input.txt file format. Width must be max of 4 characters long.");
        exit(-1);
    }

    if (strlen(inputHeight) > 4) {
        fprintf(stderr, "Improper input.txt file format. Height must be max of 4 characters long.");
        exit(-1);
    }

    char* widthEnd;
    int width = (int) strtol(inputWidth, &widthEnd, 10);
    if (strcmp("", widthEnd) != 0 && *widthEnd != 0) {
        fprintf(stderr, "Improper input.txt file format. Width must be a valid integer.");
        exit(-1);
    }


    char* heightEnd;
    int height = (int) strtol(inputHeight, &heightEnd, 10);
    if (strcmp("", heightEnd) != 0 && *heightEnd != 0) {
        fprintf(stderr, "Improper input.txt file format. Height must be a valid integer.");
        exit(-1);
    }

    printf("imsize keyword: %s\n", inputImSizeKeyword);
    printf("width: %d\n", width);
    printf("height: %d\n", height);

    char outputFileName[maxInputFileNameLength + 9];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", inputFileName, outputFileSuffix);
    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");


    fprintf(outputFilePtr, "P3\n%d %d\n255\n", width, height);

    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            fprintf(outputFilePtr, "255 222 111");
            if (w % (maxPixelsOnLine) == (maxPixelsOnLine - 1)) {
                fprintf(outputFilePtr, "\n");
            } else {
                fprintf(outputFilePtr, "\t");
            }
        }
    }

    fclose(outputFilePtr);
    return 0;
}
