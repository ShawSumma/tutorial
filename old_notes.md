# Writing a VM

This document explains how to write a fast VM.

- [Writing a VM](#writing-a-vm)
- [Choosing a Host Language](#choosing-a-host-language)
- [Example 1](#example-1)
  - [First Try Interpreter](#first-try-interpreter)
  - [First Attempt Optimizing](#first-attempt-optimizing)
  - [Second](#second)

# Choosing a Host Language

Your host language of choice will need to have some very low level features.

* It must have computed goto.
* It should not be garbage collected.
* It must be able to do untaged/raw unions.

I'll choose C for this document.

# Example 1

For this example the bytecode definition will be the following grammar

```
program: instr* exit_instr
exit_instr: "x"

instr: toggle_instr | fallthrough_instr
fallthrough: "f"
toggle_instr: "t"
inc_instr: "+"
```

## First Try Interpreter

```c
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
```

To build and run this type the following into any shell.

```sh
cc src/ex1/basic.c -o bin/ex1-basic -O3 -flto -fomit-frame-pointer
time ./bin/ex1-basic "+tttttttttttttttttttttttttttt"
```

You should see `268435456` for the output of the program, along with timing information.

On my machine it takes about 0.8 seconds. This is a good start, but we can do better.

## First Attempt Optimizing

Let's start by optimizing what we have down.

One of the first things to do is try to use packed constants.

```c
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
```

This code is much better performing, and for this example it is the fastest way to accurately interpret the code.

## Second Attempt Optimizing

This code uses computed goto, which scales much more nicely.

Notice how there is no opcode table, it references them directly as the compiler is inline to the interpeter function.

```c
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
```

This code runs about as fast as the first try optimizing, but should have better scalability due to being able to use more than 256 opcodes without changing anything.
