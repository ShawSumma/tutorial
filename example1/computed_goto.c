
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    STOP_RUNNING = '\0',
    INCREMENT_TOTAL = '+',
    TOGGLE_AND_RESTART = 't',
    TOGGLE_AND_PASS = 'f',
};

int main(int arg_count, char **arg_values) {
    for (int argument_number = 1; argument_number < arg_count; argument_number++) {
        int reg = 0;
        char *source_buffer = arg_values[argument_number];
        void **computed_goto_buffer = malloc(sizeof(void *) * (strlen(source_buffer) + 1));
        int source_index = 0;
        while (true) {
            switch (source_buffer[source_index]) {
            case STOP_RUNNING:
                computed_goto_buffer[source_index] = &&execute_stop_running;
                break;
            case INCREMENT_TOTAL:
                computed_goto_buffer[source_index] = &&execute_increment_total;
                break;
            case TOGGLE_AND_RESTART:
                computed_goto_buffer[source_index] = &&execute_toggle_and_restart;
                break;
            case TOGGLE_AND_PASS:
                computed_goto_buffer[source_index] = &&execute_toggle_and_pass;
                break;
            }
            if (source_buffer[source_index] == '\0') {
                break;
            }
            source_index += 1;
        }
        void **code = computed_goto_buffer;
        goto **code;
        execute_stop_running: {
            free(computed_goto_buffer);
            printf("%i\n", reg);
            continue;
        }
        execute_increment_total: {
            code += 1;
            reg += 1;
            goto **code;
        }
        execute_toggle_and_restart: {
            *code = &&execute_toggle_and_pass;
            code = computed_goto_buffer;
            goto **code;
        }
        execute_toggle_and_pass: {
            *code = &&execute_toggle_and_restart;
            code += 1;
            goto **code;
        }
    }
}
