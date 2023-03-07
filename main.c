#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;
typedef uint (*decode_fn)(uint8_t *image, uint offset, uint size);

enum instruction {
	INST_UNKNOWN = 0,
	/* move register/memory to/from register */
	INST_MOV_RM_REG,
	/* move immediate to register/memory */
	INST_MOV_IMM_RM,
	/* move immediate to register*/
	INST_MOV_IMM_REG,
	/* move memory to accumulator */
	INST_MOV_MEM_ACC,
	/* move accumulator to memory */
	INST_MOV_ACC_MEM,
};

static char *registers[16] =
{
	"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", // W = 0
	"ax", "cx", "dx", "bx", "sp", "bp", "si", "di", // W = 1
};

// equations used in effective address calculation
static char *ea_base[8] =
{
	"bx + si",
	"bx + di",
	"bp + si",
	"bp + di",
	"si",
	"di",
	"bp",
	"bx",
};

// instruction decoders
static uint decode_unknown    (uint8_t *image, uint offset, uint size);
static uint decode_mov_rm_reg (uint8_t *image, uint offset, uint size);
static uint decode_mov_imm_rm (uint8_t *image, uint offset, uint size);
static uint decode_mov_imm_reg(uint8_t *image, uint offset, uint size);
static uint decode_mov_mem_acc(uint8_t *image, uint offset, uint size);
static uint decode_mov_acc_mem(uint8_t *image, uint offset, uint size);

static decode_fn decoders[] =
{
	&decode_unknown,
	&decode_mov_rm_reg,
	&decode_mov_imm_rm,
	&decode_mov_imm_reg,
	&decode_mov_mem_acc,
	&decode_mov_acc_mem,
};

static enum instruction identify_inst(uint8_t *image, uint offset);

int main(int argc, char *argv[])
{
	size_t   nread;
	size_t   size;
	uint     offset;

	FILE    *asm_file;
	uint8_t *image;
	enum instruction inst;

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
		inst = identify_inst(image, offset);
		offset = decoders[inst](image, offset, size);
	}

	free(image);
	fclose(asm_file);
	return 0;
}

enum instruction identify_inst(uint8_t *image, uint offset)
{
	uint8_t *inst;

	inst = image + offset;
	// move register/memory to/from register
	if ((inst[0] >> 2) == 0b100010)
		return INST_MOV_RM_REG;
	// move immediate to register/memory
	if (((inst[0] >> 1) == 0b1100011) && !(inst[1] & 0b00111000))
		return INST_MOV_IMM_RM;
	// move immediate to register
	if ((inst[0] >> 4) == 0b1011)
		return INST_MOV_IMM_REG;
	// move memory to accumulator
	if ((inst[0] >> 1) == 0b1010000)
		return INST_MOV_MEM_ACC;
	// move accumulator to memory
	if ((inst[0] >> 1) == 0b1010001)
		return INST_MOV_ACC_MEM;

	return INST_UNKNOWN;
}

uint decode_unknown(uint8_t *image, uint offset, uint size)
{
	int i;

	(void)size;

	fprintf(stderr, "; 0x%08X: unknown opcode: ", offset);

	for (i = 7; i >= 0; --i) {
		fprintf(stderr, "%c", (image[offset] & (1U << i)) ? '1' : '0');
	}
		
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

uint decode_mov_rm_reg(uint8_t *image, uint offset, uint size)
{
	int  len;
	char ea_str[64];
	char *first_op, *second_op, *tmp_op;

	uint8_t d, w, mod, reg, r_m;
	uint8_t *inst = image + offset;
	uint16_t tmp;

	offset += 2;
	if (offset > size) {
		return offset;
	}

	d   = (inst[0] >> 1) & 0b1;
	w   = (inst[0] >> 0) & 0b1;

	mod = (inst[1] >> 6) & 0b11;
	reg = (inst[1] >> 3) & 0b111;
	r_m = (inst[1] >> 0) & 0b111;

	first_op  = registers[(w << 3) | reg];
	second_op = registers[(w << 3) | r_m];

	if (mod != 0b11) {
		tmp = (inst[3] << 8) | inst[2];
		offset += 2;

		if (mod == 0b00 && r_m == 0b110) {
			len = snprintf(ea_str, sizeof(ea_str), "[%u]", tmp);
		} else {
			if (mod == 0b01) {
				tmp &= 0x00FF;
				offset--;
			} else if (mod == 0b00) {
				tmp = 0;
				offset -= 2;
			}

			len = snprintf(ea_str, sizeof(ea_str), "[%s",
			               ea_base[r_m]);
			if (tmp > 0) {
				len += snprintf(ea_str + len,
				                sizeof(ea_str) - len, " + %u",
				                tmp);
			}

			len += snprintf(ea_str + len, sizeof(ea_str) - len,
			                "]");
		}

		second_op = ea_str;
	}

	if (!d) {
		tmp_op    = first_op;
		first_op  = second_op;
		second_op = tmp_op;
	}

	printf("mov %s, %s", first_op, second_op);
	printf(" ; d: %u, w: %u, mod: %u, reg: %u, r/m: %u, off: 0x%X", d, w, mod, reg, r_m, offset);
	printf("\n");

	return offset;
}

uint decode_mov_imm_rm(uint8_t *image, uint offset, uint size)
{
	(void)image; (void)offset; (void)size;
	return offset;
}

uint decode_mov_imm_reg(uint8_t *image, uint offset, uint size)
{
	(void)image; (void)offset; (void)size;
	return offset;
}	

uint decode_mov_mem_acc(uint8_t *image, uint offset, uint size)
{
	(void)image; (void)offset; (void)size;
	return offset;
}

uint decode_mov_acc_mem(uint8_t *image, uint offset, uint size)
{
	(void)image; (void)offset; (void)size;
	return offset;
}

