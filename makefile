
CC ?= cc
OPT ?= -O3 -fomit-frame-pointer

example-1: example1
	@$(MAKE) bin/example1/first_try bin/example1/with_compiler bin/example1/computed_goto

bin/example1/first_try: example1/first_try.c
	@mkdir -p bin/example1
	$(CC) example1/first_try.c -o $(@) $(OPT) $(CFLAGS)

bin/example1/with_compiler: example1/with_compiler.c
	@mkdir -p bin/example1
	$(CC) example1/with_compiler.c -o $(@) $(OPT) $(CFLAGS)

bin/example1/computed_goto: example1/computed_goto.c
	@mkdir -p bin/example1
	$(CC) example1/computed_goto.c -o $(@) $(OPT) $(CFLAGS)

.dummy:
