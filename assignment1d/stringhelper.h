#ifndef FUNDAMENTALS_OF_COMPUTER_GRAPHICS_STRINGHELPER_H
#define FUNDAMENTALS_OF_COMPUTER_GRAPHICS_STRINGHELPER_H

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

bool endsWith(char* string, char* ending) {
    string = strrchr(string, '.');

    if (string != NULL && strcmp(string, ending) == 0) {
        return true;
    }

    return false;
}

#endif
