
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *compile(char *arg) {
    uint8_t *ret = malloc(sizeof(uint8_t) * (strlen(arg) + 1));
    uint8_t *cur = ret;
    while (true) {
        switch (*arg) {
        case '\0':
            *cur++ = 0;
            return ret;
        case '+':
            *cur++ = 1;
            break;
        case 't':
            *cur++ = 2;
            break;
        case 'f':
            *cur++ = 3;
            break;
        }
        arg += 1;
    }
}

int main(int argc, char **argv) {
    for (int argno = 1; argno < argc; argno++) {
        int reg = 0;
        uint8_t *init = compile(argv[argno]);
        uint8_t *cur = init;
        while (true) {
            switch (*cur) {
            case 0:
                goto end;
            case 1:
                reg += 1;
                cur += 1;
                break;
            case 2:
                *cur = 3;
                cur = init;
                break;
            case 3:
                *cur = 2;
                cur += 1;
                break;
            default:
                __builtin_unreachable();
            }
        }
    end:;
        free(init);
        printf("%i", reg);
    }
}
