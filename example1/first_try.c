
#include <stdio.h>

enum {
    STOP_RUNNING = '\0',
    INCREMENT_TOTAL = '+',
    TOGGLE_AND_RESTART = 't',
    TOGGLE_AND_PASS = 'f',
};

int main(int arg_count, char **arg_values) {
    for (int argument_number = 1; argument_number < arg_count; argument_number++) {
        int accumulator_register = 0;
        char *source_string_base = arg_values[argument_number];
        char *source_string = source_string_base;
        while (*source_string != STOP_RUNNING) {
            switch (*source_string) {
            case INCREMENT_TOTAL:
                accumulator_register += 1;
                source_string += 1;
                break;
            case TOGGLE_AND_RESTART:
                *source_string = TOGGLE_AND_PASS;
                source_string = source_string_base;
                break;
            case TOGGLE_AND_PASS:
                *source_string = TOGGLE_AND_RESTART;
                source_string += 1;
                break;
            }
        }
        printf("%i\n", accumulator_register);
    }
}
