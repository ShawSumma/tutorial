
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    for (int argno = 1; argno < argc; argno++) {
        int reg = 0;
        char *arg = argv[argno];
        void **base = (void **) malloc(sizeof(void *) * (strlen(arg) + 1));
        int head = 0;
        while (true) {
            switch (arg[head]) {
            case '\0':
                base[head] = &&do_zero;
                break;
            case '+':
                base[head] = &&do_plus;
                break;
            case 't':
                base[head] = &&do_toggle;
                break;
            case 'f':
                base[head] = &&do_fall;
                break;
            }
            if (arg[head] == '\0') {
                break;
            }
            head += 1;
        }
        void **code = base;
        goto **code;
        do_zero: {
            free(base);
            printf("%i", reg);
            continue;
        }
        do_plus: {
            code += 1;
            reg += 1;
            goto **code;
        }
        do_toggle: {
            *code = &&do_fall;
            code = base;
            goto **code;
        }
        do_fall: {
            *code = &&do_toggle;
            code += 1;
            goto **code;
        }
    }
}
