#include <stdio.h>

#include "bitmap.h"
#include "decoder.h"

static char *segregs[4] = { "es", "cs", "ss", "ds" };
static char *regs[2][16] =
{
	{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" },
	{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" },
};

static const char *gen_name(enum inst_type type);

int decode_inst(FILE *out, struct inst_data data, uint8 * const image,
                uint size, uint offset)
{
	
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
		case INST_SGMNT:  return "sgmnt";
		case INST_SHL:    return "shl";
		case INST_SHR:    return "shr";
		case INST_STC:    return "stc";
		case INST_STD:    return "std";
		case INST_STDSB:  return "stdsb";
		case INST_STDSW:  return "stdsw";
		case INST_STI:    return "sti";
		case INST_SUB:    return "sub";
		case INST_TEST:   return "test";
		case INST_WAIT:   return "wait";
		case INST_XCHG:   return "xchg";
		case INST_XLAT:   return "xlat";
		case INST_XOR:    return "xor";

		case INST_UNK:
		case INST_EXTD:
			return "<invalid>";
	}

	return "<unknown>";
}

