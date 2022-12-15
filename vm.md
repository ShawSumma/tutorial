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
