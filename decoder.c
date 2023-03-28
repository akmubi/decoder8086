#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "decoder.h"
#include "inst.h"

#define W(flags) (!!((flags) & FLAG_W))

static char *segregs[4] = { "es", "cs", "ss", "ds" };
static char *regs[2][16] =
{
	{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" },
	{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" },
};

static const char *gen_name(enum inst_type type);

typedef void (*decode_fn)(FILE *, struct inst *);

static void decode_rm   (FILE *out, struct inst *inst);
static void decode_sr   (FILE *out, struct inst *inst);
static void decode_reg  (FILE *out, struct inst *inst);
static void decode_v    (FILE *out, struct inst *inst);
static void decode_imm  (FILE *out, struct inst *inst);
static void decode_acc  (FILE *out, struct inst *inst);
static void decode_dx   (FILE *out, struct inst *inst);
static void decode_imm8 (FILE *out, struct inst *inst);
static void decode_mem  (FILE *out, struct inst *inst);
static void decode_addr (FILE *out, struct inst *inst);
static void decode_naddr(FILE *out, struct inst *inst);
static void decode_faddr(FILE *out, struct inst *inst);

int decode_inst(FILE *out, struct inst *inst)
{
	decode_fn op1 = NULL, op2 = NULL, tmp;

	if (!out || !inst) {
		fprintf(stderr, "invalid arguments (out: %p, image: %p)\n", out,
		        inst);
		return -1;
	}

	if (inst->base.flags & FLAG_LB) {
		fprintf(out, "label_%u:\n", inst->offset);
	}

	fprintf(out, "%s", gen_name(inst->base.type));

	if (inst->base.prefixes & PFX_FAR) fprintf(out, " far");

	switch (inst->base.fmt) {
	case INST_FMT_RM:
		op1 = decode_rm;
		break;
	case INST_FMT_RM_V:
		op1 = decode_rm;
		op2 = decode_v;
		break;
	case INST_FMT_RM_SR:
		op1 = decode_rm;
		op2 = decode_sr;
		break;
	case INST_FMT_RM_REG:
		op1 = decode_rm;
		op2 = decode_reg;
		break;
	case INST_FMT_RM_IMM:
		op1 = decode_rm;
		op2 = decode_imm;
		break;
	case INST_FMT_RM_ESC:
		assert(0 && "not implemented");
		break;
	case INST_FMT_ACC_DX:
		op1 = decode_acc;
		op2 = decode_dx;
		break;
	case INST_FMT_ACC_IMM8:
		op1 = decode_acc;
		op2 = decode_imm8;
		break;
	case INST_FMT_ACC_IMM:
		op1 = decode_acc;
		op2 = decode_imm;
		break;
	case INST_FMT_ACC_REG:
		op1 = decode_acc;
		op2 = decode_reg;
		break;
	case INST_FMT_ACC_MEM:
		op1 = decode_acc;
		op2 = decode_mem;
		break;
	case INST_FMT_REG:
		op1 = decode_reg;
		break;
	case INST_FMT_REG_IMM:
		op1 = decode_reg;
		op2 = decode_imm;
		break;
	case INST_FMT_SR:
		op1 = decode_sr;
		break;
	case INST_FMT_IMM:
		op1 = decode_imm;
		break;
	case INST_FMT_JMP_SHORT:
		op1 = decode_addr;
		break;
	case INST_FMT_JMP_NEAR:
		op1 = decode_naddr;
		break;
	case INST_FMT_JMP_FAR:
		op1 = decode_faddr;
		break;
	case INST_FMT_NONE:
		break;
	}

	if (inst->base.flags & FLAG_D) {
		tmp = op1;
		op1 = op2;
		op2 = tmp;
	}

	if (op1) {
		fputc(' ', out);
		op1(out, inst);
	}

	if (op2) {
		fprintf(out, ", ");
		op2(out, inst);
	}

	return 0;
}

const char *gen_name(enum inst_type type)
{
	switch(type) {
		case INST_AAA:    return "aaa";
		case INST_AAD:    return "aad";
		case INST_AAM:    return "aam";
		case INST_AAS:    return "aas";
		case INST_ADC:    return "adc";
		case INST_ADD:    return "add";
		case INST_AND:    return "and";
		case INST_CALL:   return "call";
		case INST_CALLF:  return "callf";
		case INST_CBW:    return "cbw";
		case INST_CLC:    return "clc";
		case INST_CLD:    return "cld";
		case INST_CLI:    return "cli";
		case INST_CMC:    return "cmc";
		case INST_CMP:    return "cmp";
		case INST_CMPSB:  return "cmpsb";
		case INST_CMPSW:  return "cmpsw";
		case INST_CWD:    return "cwd";
		case INST_DAA:    return "daa";
		case INST_DAS:    return "das";
		case INST_DEC:    return "dec";
		case INST_DIV:    return "div";
		case INST_ESC:    return "esc";
		case INST_HLT:    return "hlt";
		case INST_IDIV:   return "idiv";
		case INST_IMUL:   return "imul";
		case INST_IN:     return "in";
		case INST_INC:    return "inc";
		case INST_INT:    return "int";
		case INST_INT3:   return "int3";
		case INST_INTO:   return "into";
		case INST_IRET:   return "iret";
		case INST_JA:     return "ja";
		case INST_JAE:    return "jae";
		case INST_JB:     return "jb";
		case INST_JBE:    return "jbe";
		case INST_JCXZ:   return "jcxz";
		case INST_JE:     return "je";
		case INST_JG:     return "jg";
		case INST_JGE:    return "jge";
		case INST_JL:     return "jl";
		case INST_JLE:    return "jle";
		case INST_JMP:    return "jmp";
		case INST_JMPF:   return "jmpf";
		case INST_JNE:    return "jne";
		case INST_JNO:    return "jno";
		case INST_JNS:    return "jns";
		case INST_JO:     return "jo";
		case INST_JP:     return "jp";
		case INST_JPO:    return "jpo";
		case INST_JS:     return "js";
		case INST_LAHF:   return "lahf";
		case INST_LDS:    return "lds";
		case INST_LEA:    return "lea";
		case INST_LES:    return "les";
		case INST_LOCK:   return "lock";
		case INST_LODSB:  return "lodsb";
		case INST_LODSW:  return "lodsw";
		case INST_LOOP:   return "loop";
		case INST_LOOPZ:  return "loopz";
		case INST_LOOPNZ: return "loopnz";
		case INST_MOV:    return "mov";
		case INST_MOVSB:  return "movsb";
		case INST_MOVSW:  return "movsw";
		case INST_MUL:    return "mul";
		case INST_NEG:    return "neg";
		case INST_NOP:    return "nop";
		case INST_NOT:    return "not";
		case INST_OR:     return "or";
		case INST_OUT:    return "out";
		case INST_POP:    return "pop";
		case INST_POPF:   return "popf";
		case INST_PUSH:   return "push";
		case INST_PUSHF:  return "pushf";
		case INST_RCL:    return "rcl";
		case INST_RCR:    return "rcr";
		case INST_REP:    return "rep";
		case INST_REPNE:  return "repne";
		case INST_RET:    return "ret";
		case INST_RETF:   return "retf";
		case INST_ROL:    return "rol";
		case INST_ROR:    return "ror";
		case INST_SAHF:   return "sahf";
		case INST_SAR:    return "sar";
		case INST_SBB:    return "sbb";
		case INST_SCASB:  return "scasb";
		case INST_SCASW:  return "scasw";
		case INST_SGMNT:  return "";
		case INST_SHL:    return "shl";
		case INST_SHR:    return "shr";
		case INST_STC:    return "stc";
		case INST_STD:    return "std";
		case INST_STOSB:  return "stosb";
		case INST_STOSW:  return "stosw";
		case INST_STI:    return "sti";
		case INST_SUB:    return "sub";
		case INST_TEST:   return "test";
		case INST_WAIT:   return "wait";
		case INST_XCHG:   return "xchg";
		case INST_XLAT:   return "xlat";
		case INST_XOR:    return "xor";

		case INST_UNK:
			return "<invalid>";
		case INST_EXTD:
			assert(0 && "INST_EXTD encountered");
	}

	return "<unknown>";
}

void decode_rm(FILE *out, struct inst *inst)
{
	int   len;
	char  ea_str[64];
	char *op;

	uint8  w, mod, r_m;

	int16 disp = *((int16 *)&inst->disp);

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

	w   = W(inst->base.flags);
	mod = FIELD_MOD(inst->fields);
	r_m = FIELD_RM(inst->fields);

	op = regs[w][r_m];

	if (mod != 0b11) {
		if (mod == 0b00 && r_m == 0b110) {
			len = snprintf(ea_str, sizeof(ea_str), "[%u]",
			               disp & 0xFFFF);
		} else {
			// [ ea_base + d8 ]
			if (mod == 0b01) {
				// only low byte
				disp &= 0x00FF;
				// if sign bit is set then sign-extend
				if (disp & 0x80) disp |= 0xFF00;

			// [ ea_base ]
			} else if (mod == 0b00) {
				// no displacement
				disp = 0;
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

		op = ea_str;

		if (inst->base.prefixes & PFX_WIDE) {
			fprintf(out, "%s ", w ? "word" : "byte");
		}

		if (inst->base.prefixes & PFX_SGMNT) {
			fprintf(out, "%s:", segregs[SGMNT_OP(inst->base.prefixes)]);
		}
	}

	fprintf(out, "%s", op);
}

void decode_reg(FILE *out, struct inst *inst)
{
	uint8 w, reg;

	w   = W(inst->base.flags);
	reg = FIELD_REG(inst->fields);

	fprintf(out, "%s", regs[w][reg]);
}

void decode_sr(FILE *out, struct inst *inst)
{
	fprintf(out, "%s", segregs[SR_OP(inst->base.flags)]);
}

void decode_v(FILE *out, struct inst *inst)
{
	fprintf(out, "%s", (inst->base.flags & FLAG_V) ? "cl" : "1");
}

void decode_imm(FILE *out, struct inst *inst)
{
	int16 imm = *((int16 *)&inst->data);
	fprintf(out, "%d", imm);
}

void decode_acc(FILE *out, struct inst *inst)
{
	fprintf(out, "%s", regs[W(inst->base.flags)][0]);
}

void decode_dx(FILE *out, struct inst *inst)
{
	(void)inst;
	fprintf(out, "dx");
}

void decode_imm8(FILE *out, struct inst *inst)
{
	fprintf(out, "%u", inst->data & 0xFF);
}

void decode_mem(FILE *out, struct inst *inst)
{
	fprintf(out, "[%u]", inst->data & 0xFFFF);
}

void decode_addr(FILE *out, struct inst *inst)
{
	int addr = get_jmp_offset(inst);
	assert(addr != -1);

	fprintf(out, "label_%d", addr);
}

void decode_naddr(FILE *out, struct inst *inst)
{
	int16_t addr = get_jmp_offset(inst);
	assert(addr != -1);

	fprintf(out, "%d", addr);
}

void decode_faddr(FILE *out, struct inst *inst)
{
	fprintf(out, "%u:%u", inst->data_ext, inst->data);
}

