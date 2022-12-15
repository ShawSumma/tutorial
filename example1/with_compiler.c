
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    STOP_RUNNING = '\0',
    INCREMENT_TOTAL = '+',
    TOGGLE_AND_RESTART = 't',
    TOGGLE_AND_PASS = 'f',
};

enum {
    COMPILED_STOP_RUNNING,
    COMPILED_INCREMENT_TOTAL,
    COMPILED_TOGGLE_AND_RESTART,
    COMPILED_TOGGLE_AND_PASS,
};

uint8_t *compile_string_to_opcode_buffer(char *source_string) {
    uint8_t *opcode_buffer = malloc(sizeof(uint8_t) * (strlen(source_string) + 1));
    uint8_t *opcode_buffer_head = opcode_buffer;
    for (int source_index = 0; source_string[source_index] != STOP_RUNNING; source_index += 1) {
        switch (source_string[source_index]) {
        case INCREMENT_TOTAL:
            *opcode_buffer_head++ = COMPILED_INCREMENT_TOTAL;
            break;
        case TOGGLE_AND_RESTART:
            *opcode_buffer_head++ = COMPILED_TOGGLE_AND_RESTART;
            break;
        case TOGGLE_AND_PASS:
            *opcode_buffer_head++ = COMPILED_TOGGLE_AND_PASS;
            break;
        }
    }
    *opcode_buffer_head++ = COMPILED_STOP_RUNNING;
    return opcode_buffer;
}

int main(int arg_count, char **arg_values) {
    for (int argument_number = 1; argument_number < arg_count; argument_number++) {
        int accumulator_register = 0;
        uint8_t *opcode_buffer_base = compile_string_to_opcode_buffer(arg_values[argument_number]);
        uint8_t *opcode_buffer_head = opcode_buffer_base;
        for (;;) {
            switch (*opcode_buffer_head) {
            case COMPILED_STOP_RUNNING:
                goto end;
            case COMPILED_INCREMENT_TOTAL:
                accumulator_register += 1;
                opcode_buffer_head += 1;
                break;
            case COMPILED_TOGGLE_AND_RESTART:
                *opcode_buffer_head = COMPILED_TOGGLE_AND_PASS;
                opcode_buffer_head = opcode_buffer_base;
                break;
            case COMPILED_TOGGLE_AND_PASS:
                *opcode_buffer_head = COMPILED_TOGGLE_AND_RESTART;
                opcode_buffer_head += 1;
                break;
            default:
                __builtin_unreachable();
            }
        }
    end:;
        free(opcode_buffer_base);
        printf("%i\n", accumulator_register);
    }
}
