# micro-ml

A compiler from **MicroML**, a small functional language, to **C**.

Written in C with a flex scanner and a hand-written recursive-descent parser and code generator, as coursework for my MSc at the Naval Postgraduate School (2007). Revised in 2026 to build with modern toolchains and to guarantee left-to-right argument evaluation.

## The language

A MicroML program is a list of function definitions followed by a goal expression. Integers are the only data type; zero is false and any other value is true.

```
FUN gcd(x, y) IF x = y THEN x
              ELSE IF x > y THEN gcd(x - y, y)
              ELSE gcd(x, y - x) FI FI;
? gcd(10, 25)
```

- Functions with parameters, recursion and nested calls
- `IF … THEN … ELSE … FI` conditional expressions
- Arithmetic `+ - * /`, comparisons `= < > <= >= !=`, logic `AND OR NOT`
- `READ_NUMBER()` reads an integer from standard input
- Comments `(* … *)`, possibly spanning several lines

The compiler reports syntax errors with line numbers, rejects a function defined twice or a parameter repeated within a function, and flags unrecognized characters and unterminated comments.

## How it works

- `micro-ml.l` is the flex scanner. It turns the source into tokens and skips comments and whitespace.
- `micro-ml.c` is the recursive-descent parser. Each grammar rule is one C function, and code generation happens while parsing.
- A function-name table and a per-function parameter table (local scope) catch duplicate definitions.
- The output is a C file plus a `microML.h` header with the function prototypes, so recursion and forward use compile cleanly.

The gcd example compiles to:

```c
int gcd(int x, int y)
{
	int _t0, _t1, _t2, _t3;
	return (x == y)? (x): ((x > y)? ((_t0 = x - y, _t1 = y, gcd(_t0, _t1))): ((_t2 = x, _t3 = y - x, gcd(_t2, _t3))));
}
```

### Argument evaluation order (2026 revision)

C leaves the order in which function arguments are evaluated unspecified. A call like `f(READ_NUMBER(), READ_NUMBER())` could therefore read its inputs right to left, and with gcc it did. Calls with two or more arguments are now emitted as a comma expression that stores each argument in a local temporary first, which forces left-to-right evaluation. Because the temporaries are locals of each generated function, the scheme is safe under recursion. `examples/input5.ml` tests this.

## Build and run

You need a C compiler and flex. On macOS both come with the Xcode command line tools.

```sh
make            # builds ./micro-ml
make test       # compiles, builds and runs every program in examples/
```

To compile your own program:

```sh
./micro-ml program.ml program.c     # writes program.c and microML.h
cc program.c -o program && ./program
```

## Examples

| File | What it does | Input | Result |
|---|---|---|---|
| `input1.ml` | adds three numbers | 3 4 5 | 12 |
| `input2.ml` | the bigger of two numbers | 9 4 | 9 |
| `input3.ml` | 1 if the inputs are 5 and 7 | 5 7 | 1 |
| `input4.ml` | recursive gcd(10, 25) | none | 5 |
| `input5.ml` | argument order under recursion | 10 1 20 2 | 27 |
