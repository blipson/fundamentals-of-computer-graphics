#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

static int maxInputFileNameLength = 100;
/*
 * 17 is the minimum number of chars required to allow for an input.txt file
 * with the maximum display resolution of 7680x4320 with this format:
                    imsize 7680 4320
*/
static int maxInputFileSize = 17;

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
    if (argc != 2) {
        fprintf(stderr, "Incorrect usage. Correct usage is `./assignment0 <path/to/input_file.txt>.");
        exit(-1);
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
    } else {
        fprintf(stderr, "Unable to open the input.txt file specified.");
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
}

void checkFileContent(char** fileContent) {
    if (strcmp(fileContent[0], "imsize") != 0) {
        fprintf(stderr, "Improper input.txt file format. The keyword 'imsize' must come first.");
        exit(-1);
    }

    if (strlen(fileContent[1]) > 4) {
        fprintf(stderr, "Improper input.txt file format. Width must be max of 4 characters long.");
        exit(-1);
    }

    if (strlen(fileContent[2]) > 4) {
        fprintf(stderr, "Improper input.txt file format. Height must be max of 4 characters long.");
        exit(-1);
    }
}

int convertStringToInt(char* s) {
    char* end;
    int i = (int) strtol(s, &end, 10);
    if (strcmp("", end) != 0 && *end != 0) {
        fprintf(stderr, "Improper input.txt file format. Width and height must both be valid integers.");
        exit(-1);
    }
    return i;
}

int main(int argc, char* argv[]) {

    static char* outputFileSuffix = "_out.ppm";
    static char* imSizeKeyword = "imsize";
    /*
     * 5 is chosen to ensure a max line length of 70 character. If each pixel
     * is the maximum character count "255 255 255" with "\t" in between,
     * it makes 12 char per pixel, meaning 5 pixels per line to stay below 70.
     */
    static int maxPixelsOnLine = 5;

    checkArgs(argc, argv);

    char inputFileContent[maxInputFileSize];
    getInputFileContent(argv[1], inputFileContent);

    char* fileContent[3];
    readFileContent(inputFileContent, fileContent);
    checkFileContent(fileContent);

    int width = convertStringToInt(fileContent[1]);
    int height = convertStringToInt(fileContent[2]);

    printf("imsize keyword: %s\n", fileContent[0]);
    printf("width: %d\n", width);
    printf("height: %d\n", height);

    char outputFileName[maxInputFileNameLength + 9];
    snprintf(outputFileName, sizeof(outputFileName), "%s%s", argv[1], outputFileSuffix);
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
