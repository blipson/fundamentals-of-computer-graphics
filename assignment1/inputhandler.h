#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUTHANDLER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_INPUTHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <_ctype.h>

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

void freeInput(char*** inputFileWordsByLine) {
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

#endif