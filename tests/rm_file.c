#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s FILE\n\tremoves a file\n", argv[0]);
		return 1;
	}

	return remove(argv[1]);
}

