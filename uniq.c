#ifndef UNIQ_C
#define UNIQ_C
#include "types.h"
#include "user.h"
typedef struct Line {
    char* str;
    int count ;
    int hashCode;
} Line;

const int _C = 1;
const int _D = 1 << 1;
const int _I = 1 << 2;
typedef int bool;

int PARAM_MASK = 0;
int lineNumber = 0;



#define  LIST_SIZE 512
#define BUFFER_SIZE 1000000

int listSize = 2;

const int BASE = 100000;
struct Line** lines;
char str_buffer[BUFFER_SIZE];
short read_buffer_index = 0;
bool foundLine = 0;

void strcopy(char* dest, char* src, int len) {
    memmove(dest, src, len );
    dest[len] = '\0';
}

char tolower(char ch) {
    char result = ch;
    if (ch >= 'A' && ch <= 'Z') {
        result = 'a' + ch - 'A';
    }
    return result;
}

bool readline(int fd, char* str, int* len) {
    int curr = read_buffer_index;
    for (; read_buffer_index < BUFFER_SIZE; read_buffer_index++) {
        char val = str_buffer[read_buffer_index];
        if (val == '\n') {
            read_buffer_index ++;
            foundLine = 1;
            break;
        }
        else if (val == '\0' || val == '\r') {
            foundLine = 0;
            strcopy(str, str_buffer + curr, read_buffer_index - curr );
            *len = read_buffer_index - curr;
            break;
        }
    }
    if (read_buffer_index == BUFFER_SIZE - 1) {
        foundLine = 0;
    }

    if (foundLine) {
        strcopy(str, str_buffer + curr, read_buffer_index - curr);
        *len = read_buffer_index - curr;
        if (*len == 1 &&
            (strcmp(str, "\r") == 0 ||
             strcmp(str, "\t") == 0 ||
             strcmp(str, "\n") == 0)) {
            foundLine = 0;
        }
    }

    return foundLine;
}

int hashCode(char* str) {
    int len = sizeof(str);
    int power = 33;
    int hash = 0;
    int i;
    for (i = 0; i < len; ++i) {
        hash = (power * hash + tolower(str[i])) % BASE;
    }
    return hash % 512;
}

Line* createLine( char* str, int len) {
    Line* line = malloc(sizeof(Line));
    line->str = malloc(len + 1);
    line->count = 1;
    line->hashCode = hashCode(str);
    memmove(line->str, str, len);
    line->str[len] = '\0';
    return line;
}

bool compare(Line* l1, Line* l2) {
    bool ignoreCase = ((PARAM_MASK & _I) > 0);
    bool equal = 1;
    int len1 = strlen(l1->str);
    int len2 = strlen(l2->str);
    if (len1 == len2 && l1->hashCode == l2->hashCode) {
        int i = 0;
        for (; i < len1; i++) {
            char ch1 = ignoreCase > 0 ? tolower(l1->str[i]) : l1->str[i];
            char ch2 = ignoreCase > 0 ? tolower(l2->str[i]) : l2->str[i];

            if (ch1 != ch2) {
                equal = 0;
                break;
            }
        }
    } else {
        equal = 0;
    }
    return equal;
}

bool contains(Line* line) {
    int i = 0;
    for (; i < lineNumber; i++) {
        if (compare(lines[i], line)) {
            lines[i]->count++;
            return 1;
        }
    }
    return 0;
}


void insert(Line* newLine) {
    if (lineNumber == listSize) {
        int oldSize = listSize;
        listSize *= 2;
        Line** newLines = (Line**)malloc(listSize * sizeof(Line));
        memmove(newLines, lines, oldSize * sizeof(Line));
        free(lines);
        lines = newLines;
    }
    lines[lineNumber++] = newLine;
}

void deleteLine(Line* line) {
    if (!line) return;
    free(line->str);
    free(line);
}

void printLines() {
    int i = 0;
    for (; i < listSize; i++) {
        if ((PARAM_MASK & _C) > 0) {
            printf(1, "%d %s ", lines[i]->count, lines[i]->str);
        } else if (((PARAM_MASK & _D) > 0)) {
            if (lines[i]->count > 1){
                printf(1, "%s ", lines[i]->str);
            }
        } else {
            printf(1, "%s ", lines[i]->str);
        }
    }
}


void
uniq(int fd) {
    int r;
    char* buffer = malloc(BUFFER_SIZE);
    if ((r = read(fd, str_buffer, sizeof(str_buffer))) > 0) {
        int len = 0;
        while (readline(fd, buffer, &len)) {
            Line* line = createLine(buffer, len);
            if (contains(line)) {
                deleteLine(line);
            } else {
                insert(line);
            }
        }
    }
    free(buffer);
}


void destroy() {
    int i;
    for (i = 0; i < listSize; i++) {
        deleteLine(lines[i]);
    }
    free(lines);
}

void initialize() {
    lines = (Line**)malloc(listSize * sizeof(Line));
}

int
main(int argc, char *argv[]) {

    if(argc <= 1){
        uniq(0);
    }

    initialize();

    int i, fd = 0;
    for (i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-c") == 0) {
            PARAM_MASK |= _C;
        }
        else if (strcmp(argv[i], "-d") == 0) {
            PARAM_MASK |= _D;
        }
        else if (strcmp(argv[i], "-i") == 0) {
            PARAM_MASK |= _I;
        } else {
            fd = open(argv[i],0);
            if (fd < 0) {
                printf(1, "Can't open file %s\n", argv[i]);
                exit();
            }
            int c = (PARAM_MASK & _C) > 0;
            int d = (PARAM_MASK & _D) > 0;
            if (c > 0 && d > 0) {
                printf(1, "Invalid Param. -c, -d can't appear at the same time.\n");
            }
            uniq(fd);
            break;

        }
    }

    destroy();
}

#endif