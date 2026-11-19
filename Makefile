# MicroML compiler: build with `make`, run the examples with `make test`
CC      ?= cc
CFLAGS  ?= -Wall -O2
LEX     ?= flex

# Disable make's built-in rules: they would regenerate micro-ml.c from micro-ml.l
.SUFFIXES:
MAKEFLAGS += --no-builtin-rules

micro-ml: lex.yy.c micro-ml.c micro-ml.h
	$(CC) $(CFLAGS) lex.yy.c micro-ml.c -o $@

lex.yy.c: micro-ml.l micro-ml.h
	$(LEX) micro-ml.l

test: micro-ml
	./run-examples.sh

clean:
	rm -rf micro-ml lex.yy.c build

.PHONY: test clean
