#include "inputoutputhandler.h"

int main(int argc, char* argv[]) {
    checkArgs(argc, argv);

    char*** inputFileWordsByLine = readInputFile(argv[1]);
    int line = 0;
    while (inputFileWordsByLine[line][0] != NULL) {
        printf("[");
        int word = 0;
        while (inputFileWordsByLine[line][word] != NULL) {
            printf("%s,", inputFileWordsByLine[line][word]);
            word++;
        }
        printf("]");
        printf("\n");
        line++;
    }

    freeInput(inputFileWordsByLine);


    exit(0);
}
