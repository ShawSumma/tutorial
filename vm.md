# A Virtual Machine

The virtual machine we will be implementing is very simple.

It has no memory and only one regiser, and only 4 distinct instructions.

# Registers

There is only one register, the [Accumulator Register](#accumulator-register)

## Accumulator Register

The accumulator register starts at value `0`.

The only instruction that does anything to the Accumulator Register is [Increment Accumulator Register](#increment-accumulator-register).

# Opcodes

* `t` = [Replace Self with `f` and Restart](#toggle-and-restart)
* `f` = [Replace Self with `t` and Continue](#toggle-and-continue)
* `+` = [Increment Accumulator Register](#increment-the-accumulator)

## Toggle and Restart

This opcode replaces itself with the [Toggle and Continue](#toggle-and-continue) opcode.

It then sets the place to interpret from to the beginning of the code.

- does not reset the [accumulator](#accumulator-register) to 0
- represented by the source code: `t`

## Toggle and Continue

This opcode replaces itself with the [Toggle and Restart](#toggle-and-restart) opcode.

It then moves on to the next instruction.

- represented by the source code: `f`

## Increment Accumulator Register

This opcode 1 to the [Accumulator Register](#accumulator-register).

- represented by the source code: `+`

## end of opcodes

This opcode prints the value of the [Accumulator Register](#accumulator-register) and shuts down the virtual machine.

- represented by the null character in source code.
    - C strings have a null character after them by default.

# First Implementation

For the first implementation simplicity is king.

```c
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
            default:
                __builtin_unreachable();
            }
        }
        printf("%i\n", accumulator_register);
    }
}
```

It uses no speical speed tricks, other than making the default case not have to be handled by the compiler.

# Second Interpreter

For the second interpreter the approach will be to change the expensive check from a sparse switch, into a number `0`, `1`, `2`, or `3`.
This is because in C and other low level languages, branches like switch are more expensive when they are sparse.

```c
int thing1(void);
int thing2(void);
int thing3(void);
int thing4(void);
int thing5(void);

int cases(int some_value) {
    switch (some_value) {
    case '\0':
        thing1();
        break;
    case 's':
        thing2();
        break;
    case 'h':
        thing3();
        break;
    case 'a':
        thing4();
        break;
    case 'w':
        thing5();
        break;
    default:
        __builtin_unreachable();
    }
}
```

Using [godbolt](https://godbolt.org/) I see that on x86 the above C compiles into the below assembly.

```x86asm
cases:
        cmp     edi, 104
        je      .L2
        jle     .L11
        cmp     edi, 115
        je      .L6
        jmp     thing5
.L11:
        test    edi, edi
        je      .L4
        jmp     thing4
.L2:
        jmp     thing3
.L4:
        jmp     thing1
.L6:
        jmp     thing2
```

Notice all the instructions `je`, `jle`, and the second `je` ? They are all jumps, which are often slow.

The following C fixes this issue.

```c
int thing1(void);
int thing2(void);
int thing3(void);
int thing4(void);
int thing5(void);

int cases(int some_value) {
    switch (some_value) {
    case 0:
        thing1();
        break;
    case 1:
        thing2();
        break;
    case 2:
        thing3();
        break;
    case 3:
        thing4();
        break;
    case 4:
        thing5();
        break;
    default:
        __builtin_unreachable();
    }
}
```

Using [godbolt](https://godbolt.org/) I see that on x86 the above C compiles into the below assembly.

```x86asm
cases:
        mov     edi, edi
        jmp     [QWORD PTR .L4[0+rdi*8]]
.L4:
        .quad   .L8
        .quad   .L7
        .quad   .L6
        .quad   .L5
        .quad   .L3
.L5:
        jmp     thing4
.L6:
        jmp     thing3
.L7:
        jmp     thing2
.L8:
        jmp     thing1
.L3:
        jmp     thing5
```

It uses a jump table and a single `jmp` with an index into the table. This is slightly slower than a single `je` or `jle` but not two or three.

## Getting the Compiler to use Jump Tables

Using this knowledge we can get the compiler to use a jump table for our switch case.

The following C generally optimizes better.

```c
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
```

This nets some performance gain when using PGO builds, as the compiler wrongly thinks we do not want a jump table on some architectures.

# Third Intepreter - Under Construction

Uses computed goto. 

```c
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
```
