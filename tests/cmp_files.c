#include <stdio.h>

int main(int argc, char* argv[])
{
	FILE *file1, *file2;
	char b1, b2;
	if (argc != 3) {
		printf("Usage: %s FILE1 FILE2\n\tcompares two files\n",
		       argv[0]);
		return 1;
	}

	file1 = fopen(argv[1], "rb");
	if (!file1) {
		printf("error: couldn't open '%s'\n", argv[1]);
		return 2;
	}

	file2 = fopen(argv[2], "rb");
	if (!file2) {
		printf("error: couldn't open '%s'\n", argv[2]);
		return 3;
	}

	do {
		b1 = fgetc(file1);
		b2 = fgetc(file2);
		if (b1 != b2) {
			printf("difference at 0x%lX [0x%02X != 0x%02X]\n",
			        ftell(file1), b1 & 0xFF, b2 & 0xFF);
			return 1;
		}
	} while (b1 != EOF && b2 != EOF);

	puts("no difference");
	return 0;
}

