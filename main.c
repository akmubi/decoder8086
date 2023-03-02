#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

typedef uint (*decode_fn)(uint8_t *image, uint offset, uint size);

static uint decoder_mov_rm(uint8_t *image, uint offset, uint size);
static uint decoder_unknown(uint8_t *image, uint offset, uint size);

static char *registers[16] =
{
	"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", // W = 0
	"ax", "cx", "dx", "bx", "sp", "bp", "si", "di", // W = 1
};

static decode_fn opcode_decoders[64] =
{
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,

	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,

	&decoder_unknown, &decoder_unknown, &decoder_mov_rm,  &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,

	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
	&decoder_unknown, &decoder_unknown, &decoder_unknown, &decoder_unknown,
};

int main(int argc, char *argv[])
{
	uint8_t  opcode;
	size_t   nread;
	size_t   size;
	uint     offset;

	FILE    *asm_file;
	uint8_t *image;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s FILE\n\twhere FILE is a 8086 "
		        "assembly file to decode\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	asm_file = fopen(argv[1], "rb");
	if (asm_file == NULL) {
		perror("failed to open file");
		exit(EXIT_FAILURE);
	}

	// determine file size
	fseek(asm_file, 0, SEEK_END);
	size = ftell(asm_file);
	rewind(asm_file);

	image = malloc(size);
	if (image == NULL) {
		perror("failed to allocate memory for a file image");
		exit(EXIT_FAILURE);
	}

	// copy file contents
	nread = fread(image, 1, size, asm_file);
	if (nread != size) {
		fprintf(stderr, "failed to read file contents");
		exit(EXIT_FAILURE);
	}

	printf("; %s\n", argv[1]);
	printf("\nbits 16\n\n");

	// decode contents
	offset = 0;
	while (offset < size) {
		opcode = image[offset] >> 2;
		// process intruction and set offset to the next
		offset = opcode_decoders[opcode](image, offset, size);
	}

	free(image);
	fclose(asm_file);
	return 0;
}

uint decoder_mov_rm(uint8_t *image, uint offset, uint size)
{
	uint8_t d, w, mod, reg, r_m;
	uint8_t dest, src;
	uint8_t *op = image + offset;

	if (offset + 2 > size) {
		goto finish_decoder_mov_rm;
	}

	d   = (op[0] >> 1) & 0b1;
	w   = op[0]        & 0b1;
	mod = (op[1] >> 6) & 0b111;
	reg = (op[1] >> 3) & 0b111;
	r_m = op[1]        & 0b111;

	if (mod != 0b11) {
		fprintf(stderr, "; 0x%08X: MOD = %02u, not supported (yet)\n",
		        offset, (mod >> 1) * 10 + (mod & 1) * 1);
		goto finish_decoder_mov_rm;
	}

	// determine which field is destination and which is source
	src = dest = (w << 3);
	if (d) {
		dest |= reg;
		src  |= r_m;
	} else {
		dest |= r_m;
		src  |= reg;
	}

	printf("mov %s, %s\n", registers[dest], registers[src]);

finish_decoder_mov_rm:
	return offset + 2;
}

uint decoder_unknown(uint8_t *image, uint offset, uint size)
{
	int i;

	(void)size;

	fprintf(stderr, "; 0x%08X: unknown opcode: ", offset);

	for (i = 7; i >= 2; --i) {
		fprintf(stderr, "%c", (image[offset] & (1U << i)) ? '1' : '0');
	}

	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

