#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

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

    char inputImSizeKeyword[6];
    char width[4];
    char height[4];

    int i = 0;
    while (inputFileContents[i] != '\n' && inputFileContents[i] != '\t' && inputFileContents[i] != ' ') {
        inputImSizeKeyword[i] = inputFileContents[i];
        i++;
    }
    inputImSizeKeyword[i] = '\0';

    // skip whitespace character.
    i++;

//    while (inputFileContents[i] != '\n' && inputFileContents[i] != '\t' && inputFileContents[i] != ' ') {
//        printf("%c\n", inputFileContents[i]);
//        width[i] = inputFileContents[i];
//        i++;
//    }
//    width[i] = '\0';
//
//    // skip whitespace character.
//    i++;
//
//    while (inputFileContents[i] != '\n' && inputFileContents[i] != '\t' && inputFileContents[i] != ' ') {
//        height[i] = inputFileContents[i];
//        i++;
//    }
//    height[i] = '\0';

    if (strcmp(inputImSizeKeyword, "imsize") != 0) {
        fprintf(stderr, "Improper input file format. The keyword 'imsize' must come first.");
    }

    printf("imsize keyword: %s\n", inputImSizeKeyword);
    printf("width: %s\n", width);
    printf("height: %s\n", height);

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
