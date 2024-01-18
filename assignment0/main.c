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
    static char* outputFileSuffix = "_out.ppm";
    static char* imSizeKeyword = "imsize";

    char* inputFileName = argv[1];

    if (strlen(inputFileName) > maxInputFileNameLength) {
        fprintf(stderr, "Input file name specified is too long. It must be under %d characters.", maxInputFileNameLength);
        exit(-1);
    }

    /* 17 is the minimum number of chars required to allow for an input file
     * with the maximum display resolution of 7680x4320 with this format:
                        imsize 7680 4320
    */
    static int maxInputFileSize = 17;
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
        fprintf(stderr, "Unable to open the input file specified.");
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
        fprintf(stderr, "Improper input file format. The keyword 'imsize' must come first.");
    }

    int width = (int) strtol(inputWidth, (char **)NULL, 10);
    int height = (int) strtol(inputHeight, (char **)NULL, 10);

    printf("imsize keyword: %s\n", inputImSizeKeyword);
    printf("width: %d\n", width);
    printf("height: %d\n", height);

    char outputFileName[maxInputFileNameLength + 9];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", inputFileName, outputFileSuffix);

    FILE* outputFilePtr;
    outputFilePtr = fopen(outputFileName, "w");

    fprintf(outputFilePtr, "P3 4 4 15\n"
                  "0  0  0    0  0  0    0  0  0   15  0 15\n"
                  " 0  0  0    0 15  7    0  0  0    0  0  0\n"
                  " 0  0  0    0  0  0    0 15  7    0  0  0\n"
                  "15  0 15    0  0  0    0  0  0    0  0  0\n");
    fclose(outputFilePtr);
    return 0;
}
