
#include <stdio.h>

int main(int argc, char **argv) {
    for (int argno = 1; argno < argc; argno++) {
        int reg = 0;
        char *arg = argv[argno];
        while (*arg != '\0') {
            switch (*arg) {
            case '+':
                reg += 1;
                arg += 1;
                break;
            case 't':
                *arg = 'f';
                arg = argv[argno];
                break;
            case 'f':
                *arg = 't';
                arg += 1;
                break;
            }
        }
        printf("%i", reg);
    }
}
