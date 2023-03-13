# decoder8086

To build the program, simply compile the `main.c` file. To test program you can use assembly source codes in `tests` directory.

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
build/main.out build/tests/0000.asm.out > build/tests/0000.asm.gen
nasm build/tests/0000.asm.gen -o build/tests/0000.asm.gen.out
diff -q build/tests/0000.asm.out build/tests/0000.asm.gen.out
```

2. Using Makefile (Unix)
```
make test
```
