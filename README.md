# decoder8086

To build the program, simply compile the `main.c` file. The `tests` folder contains assembly source codes for basic instructions that can be used to test the program. To compile the tests, use `nasm` and then feed them to the program.

## Building
1. By yourself:
```
mkdir build
gcc main.c bitmap.c -o build/main.out
```
2. Using Makefile (Unix)
```
make
```

## Testing
1. By yourself:
```
mkdir build/tests
nasm tests/0000.asm -o build/tests/0000.asm.out
build/main.out build/tests/0000.asm.out
```

2. Using Makefile (Unix)
```
make test
```
