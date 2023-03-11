#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;
typedef uint (*decode_fn)(uint8_t *image, uint offset);

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

// used in effective address calculation
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
static uint decode_unknown    (uint8_t *image, uint offset);
static uint decode_mov_rm_reg (uint8_t *image, uint offset);
static uint decode_mov_imm_rm (uint8_t *image, uint offset);
static uint decode_mov_imm_reg(uint8_t *image, uint offset);
static uint decode_mov_mem_acc(uint8_t *image, uint offset);
static uint decode_mov_acc_mem(uint8_t *image, uint offset);

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
	int      rc = 0;
	size_t   nread;
	size_t   size;
	uint     offset;

	FILE    *asm_file;
	uint8_t *image;
	enum instruction inst;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s FILE\n\twhere FILE is a 8086 "
		        "assembly file to decode\n", argv[0]);
		rc = 1;
		goto exit_main;
	}

	asm_file = fopen(argv[1], "rb");
	if (asm_file == NULL) {
		perror("failed to open file");
		rc = 2;
		goto exit_main;
	}

	// determine file size
	fseek(asm_file, 0, SEEK_END);
	size = ftell(asm_file);
	rewind(asm_file);

	image = malloc(size);
	if (image == NULL) {
		perror("failed to allocate memory for a file image");
		rc = 3;
		goto fclose_file;
	}

	// copy file contents
	nread = fread(image, 1, size, asm_file);
	if (nread != size) {
		fprintf(stderr, "failed to read file contents");
		rc = 4;
		goto free_image;
	}

	printf("; %s\n", argv[1]);
	printf("\nbits 16\n\n");

	// decode contents
	offset = 0;
	while (offset < size) {
		inst = identify_inst(image, offset);
		offset = decoders[inst](image, offset);
	}

free_image:
	free(image);
fclose_file:
	fclose(asm_file);
exit_main:
	return rc;
}

enum instruction identify_inst(uint8_t *image, uint offset)
{
	uint8_t *inst;

	inst = image + offset;
	if ((inst[0] >> 2) == 0b100010)
		return INST_MOV_RM_REG;
	if (((inst[0] >> 1) == 0b1100011) && !(inst[1] & 0b00111000))
		return INST_MOV_IMM_RM;
	if ((inst[0] >> 4) == 0b1011)
		return INST_MOV_IMM_REG;
	if ((inst[0] >> 1) == 0b1010000)
		return INST_MOV_MEM_ACC;
	if ((inst[0] >> 1) == 0b1010001)
		return INST_MOV_ACC_MEM;

	return INST_UNKNOWN;
}

uint decode_unknown(uint8_t *image, uint offset)
{
	int i;

	fprintf(stderr, "; 0x%08X: unknown opcode: ", offset);

	for (i = 7; i >= 0; --i) {
		fprintf(stderr, "%c", (image[offset] & (1U << i)) ? '1' : '0');
	}

	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

uint decode_mov_rm_reg(uint8_t *image, uint offset)
{
	int   len;
	char  ea_str[64];
	char *first_op, *second_op, *tmp_op;

	uint     inst_size;
	uint8_t  d, w, mod, reg, r_m;
	uint8_t *inst = image + offset;
	int16_t disp;

	inst_size = 2;

	d   = (inst[0] >> 1) & 0b1;
	w   = (inst[0])      & 0b1;

	mod = (inst[1] >> 6) & 0b11;
	reg = (inst[1] >> 3) & 0b111;
	r_m = (inst[1])      & 0b111;

	first_op  = registers[(w << 3) | reg];
	second_op = registers[(w << 3) | r_m];

	if (mod != 0b11) {
		disp = (inst[3] << 8) | inst[2];
		inst_size += 2;

		if (mod == 0b00 && r_m == 0b110) {
			len = snprintf(ea_str, sizeof(ea_str), "[%u]", disp);
		} else {
			// [ ea_base + d8 ]
			if (mod == 0b01) {
				// only low byte
				disp &= 0x00FF;
				// if sign bit is set then sign-extend
				if (disp & 0x80) {
					disp |= 0xFF00;
				}

				inst_size--;
			// [ ea_base ]
			} else if (mod == 0b00) {
				// no displacement
				disp = 0;
				inst_size -= 2;
			}

			len = snprintf(ea_str, sizeof(ea_str), "[%s",
			               ea_base[r_m]);
			if (disp != 0) {
				len += snprintf(ea_str + len,
				                sizeof(ea_str) - len, " %c %d",
				                (disp < 0) ? '-' : '+',
				                abs(disp));
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

	printf("mov %s, %s\n", first_op, second_op);

	return offset + inst_size;
}

uint decode_mov_imm_rm(uint8_t *image, uint offset)
{
	int   len;
	char  ea_str[64];
	char *dest_op, *imm_size;

	uint8_t  w, mod, r_m;
	uint8_t *inst = image + offset;
	uint16_t imm;
	int16_t  disp;

	offset += 2;

	w   = (inst[0])      & 0b1;
	mod = (inst[1] >> 6) & 0b11;
	r_m = (inst[1])      & 0b111;

	dest_op  = registers[(w << 3) | r_m];
	imm      = (inst[3] << 8) * w | inst[2];
	imm_size = ""; // if destination is register size can be ommited

	if (mod != 0b11) {
		imm_size = (w) ? "word " : "byte ";

		disp = (inst[3] << 8) | inst[2];
		offset += 2;

		if (mod == 0b00 && r_m == 0b110) {
			len = snprintf(ea_str, sizeof(ea_str), "[%u]", disp);
		} else {
			if (mod == 0b01) {
				disp &= 0x00FF;
				if (disp & 0x80) {
					disp |= 0xFF00;
				}

				offset--;
			} else if (mod == 0b00) {
				disp = 0;
				offset -= 2;
			}

			len = snprintf(ea_str, sizeof(ea_str), "[%s",
			               ea_base[r_m]);
			if (disp != 0) {
				len += snprintf(ea_str + len,
				                sizeof(ea_str) - len, " %c + %d",
				                (disp < 0) ? '-' : '+',
				                abs(disp));
			}

			len += snprintf(ea_str + len, sizeof(ea_str) - len,
			                "]");
		}

		dest_op = ea_str;

		// immediate is placed after displacement value
		imm = (image[offset + 1] << 8) * w | image[offset];
		offset += 1 + w;
	}

	printf("mov %s, %s%u\n", dest_op, imm_size, imm);

	return offset;
}

uint decode_mov_imm_reg(uint8_t *image, uint offset)
{
	uint8_t w, reg;
	uint8_t *inst = image + offset;
	uint16_t imm;

	w   = (inst[0] >> 3) & 0b1;
	reg = (inst[0])      & 0b111;

	imm = (inst[2] << 8) * w | inst[1];

	printf("mov %s, %u\n", registers[(w << 3) | reg], imm);

	return offset + 2 + w;
}	

uint decode_mov_mem_acc(uint8_t *image, uint offset)
{
	uint8_t w;
	uint8_t *inst = image + offset;
	uint16_t mem_addr;

	w = inst[0] & 0b1;

	mem_addr = (inst[2] << 8) * w | inst[1];

	printf("mov ax, [%u]\n", mem_addr);

	return offset + 2 + w;
}

uint decode_mov_acc_mem(uint8_t *image, uint offset)
{
	uint8_t w;
	uint8_t *inst = image + offset;
	uint16_t mem_addr;

	w = inst[0] & 0b1;

	mem_addr = (inst[2] << 8) * w | inst[1];

	printf("mov [%u], ax\n", mem_addr);

	return offset + 2 + w;
}

