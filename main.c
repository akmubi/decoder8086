#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"

#define FIELD_Z(byte)    (((byte) >> 0) & 0b1)
#define FIELD_W(byte)    (((byte) >> 0) & 0b1)
#define FIELD_D(byte)    (((byte) >> 1) & 0b1)
#define FIELD_V(byte)    (((byte) >> 1) & 0b1)
#define FIELD_S(byte)    (((byte) >> 1) & 0b1)
#define FIELD_SR(byte)   (((byte) >> 3) & 0b11)
#define FIELD_MOD(byte)  (((byte) >> 6) & 0b11)
#define FIELD_RM(byte)   (((byte) >> 0) & 0b111)
#define FIELD_REG(byte)  (((byte) >> 3) & 0b111)
// used to defined registers al-bh and ax-di
#define FIELD_REG2(byte) (((byte) >> 0) & 0b111)

enum inst_type
{
	INST_TYPE_UNK = 0,

	/* ============================DATA TRANSFER========================= */

	// move
	INST_TYPE_MOV,
	// push
	INST_TYPE_PUSH,
	// pop
	INST_TYPE_POP,
	// exchange
	INST_TYPE_XCHG,
	// input from
	INST_TYPE_IN,
	// output to
	INST_TYPE_OUT,
	// translate byte to al
	INST_TYPE_XLAT,
	// load effective address to register
	INST_TYPE_LEA,
	// load pointer to ds
	INST_TYPE_LDS,
	// load pointer to es
	INST_TYPE_LES,
	// load ah with flags
	INST_TYPE_LAHF,
	// store ah with flags
	INST_TYPE_SAHF,
	// push flags
	INST_TYPE_PUSHF,
	// pop flags
	INST_TYPE_POPF,

	/* ==============================ARITHMETIC========================== */

	// add
	INST_TYPE_ADD,
	// add with carry
	INST_TYPE_ADC,
	// increment
	INST_TYPE_INC,
	// ASCII adjust for add
	INST_TYPE_AAA,
	// Decimal adjust for add
	INST_TYPE_DAA,
	// substract
	INST_TYPE_SUB,
	// substract with borrow
	INST_TYPE_SBB,
	// decrement
	INST_TYPE_DEC,
	// change sign
	INST_TYPE_NEG,
	// compare
	INST_TYPE_CMP,
	// ASCII adjust for substract
	INST_TYPE_AAS,
	// decimal adjust for substract
	INST_TYPE_DAS,
	// multiply (unsigned)
	INST_TYPE_MUL,
	// integer multiply (signed)
	INST_TYPE_IMUL,
	// ASCII adjust for multiply
	INST_TYPE_AAM,
	// divide (unsigned)
	INST_TYPE_DIV,
	// integer divide (signed)
	INST_TYPE_IDIV,
	// ASCII adjust for divide
	INST_TYPE_AAD,
	// convert byte to word
	INST_TYPE_CBW,
	// convert word to double word
	INST_TYPE_CWD,

	/* ================================LOGIC============================= */

	// invert
	INST_TYPE_NOT,
	// shift logical/arithmetic left
	INST_TYPE_SHL,
	// shift logical right
	INST_TYPE_SHR,
	// shift arithmetic right
	INST_TYPE_SAR,
	// rotate left
	INST_TYPE_ROL,
	// rotate right
	INST_TYPE_ROR,
	// rotate through carry flag left
	INST_TYPE_RCL,
	// rotate through carry flag right
	INST_TYPE_RCR,
	// and
	INST_TYPE_AND,
	// and function to flags no result
	INST_TYPE_TEST,
	// or
	INST_TYPE_OR,
	// exclusive or
	INST_TYPE_XOR,

	/* =======================STRING MANIPULATION======================== */

	// repeat/loop while zero flag is clear
	INST_TYPE_REPNZ,
	// repeat/loop while zero flag is set
	INST_TYPE_REPZ,
	// move byte/word
	INST_TYPE_MOVS,
	// compare byte/word
	INST_TYPE_CMPS,
	// scan byte/word
	INST_TYPE_SCAS,
	// load byte/word to al/ax
	INST_TYPE_LODS,
	// store byte/word to al/ax
	INST_TYPE_STDS,

	/* ========================CONTROL TRANSFTER========================= */

	// call
	INST_TYPE_CALL,
	// unconditional jump
	INST_TYPE_JMP,
	// return from call
	INST_TYPE_RET,
	// jump on equal/zero
	INST_TYPE_JE,
	// jump on less/not greater or equal
	INST_TYPE_JL,
	// jump on less or equal/not greater
	INST_TYPE_JLE,
	// jump on below/not above or equal
	INST_TYPE_JB,
	// jump on below or equal/not above
	INST_TYPE_JBE,
	// jump on parity/parity even
	INST_TYPE_JP,
	// jump on overflow
	INST_TYPE_JO,
	// jump on sign
	INST_TYPE_JS,
	// jump on not equal/not zero
	INST_TYPE_JNE,
	// jump on not less/greater or equal
	INST_TYPE_JGE,
	// jump on not less or equal/greater
	INST_TYPE_JG,
	// jump on not below/above or equal
	INST_TYPE_JAE,
	// jump on not below or equal/above
	INST_TYPE_JA,
	// jump on not parity/parity odd
	INST_TYPE_JNP,
	// jump on not overflow
	INST_TYPE_JNO,
	// jump on not sign
	INST_TYPE_JNS,
	// loop cx times
	INST_TYPE_LOOP,
	// loop while zero/equal
	INST_TYPE_LOOPE,
	// loop while not zero/equal
	INST_TYPE_LOOPNE,
	// jump on cx zero
	INST_TYPE_JCXZ,
	// interrupt
	INST_TYPE_INT,
	// interrupt on overflow
	INST_TYPE_INTO,
	// interrupt return
	INST_TYPE_IRET,

	/* ========================PROCESSOR CONTROL========================= */

	// clear carry
	INST_TYPE_CLC,
	// complement carry
	INST_TYPE_CMC,
	// set carry
	INST_TYPE_STC,
	// clear direction
	INST_TYPE_CLD,
	// set direction
	INST_TYPE_STD,
	// clear interrupt
	INST_TYPE_CLI,
	// set interrupt
	INST_TYPE_STI,
	// halt
	INST_TYPE_HLT,
	// wait
	INST_TYPE_WAIT,
	// escape (to external device)
	INST_TYPE_ESC,
	// bus lock prefix
	INST_TYPE_LOCK,
	// segment override
	INST_TYPE_SGMNT,

	// for opcodes that use extra bits to encode intruction
	// they're handled separately
	INST_TYPE_EXTD,
};

typedef unsigned int uint;
typedef void (*decode_fn)(uint8_t *image, uint offset);

// returns appropriate instruction type, size and operand decoders
static struct inst_data get_inst_data(uint8_t * const image, uint offset);
// generates mnemonic for instruction
static const char *gen_inst_name(enum inst_type type, uint8_t * const image,
                                 uint offset);
// determine the label if instruction has one
static int get_label_addr(enum inst_type type, uint8_t * const image,
                            uint offset);

// calculate displacement size for reg/mem field
static uint calc_disp_size(uint8_t * const image, uint offset);

// register/memory (from mod, r/m and disp fields)
static void decode_reg_mem(uint8_t * const image, uint offset);
// accumulator (al or ax)
static void decode_acc(uint8_t * const image, uint offset);
// immediate unsigned value for register/memory (from data-lo/data-hi fields)
static void decode_imm(uint8_t * const image, uint offset);
// immediate signed value for register/memory (from data-sx field)
static void decode_imm_sx(uint8_t * const image, uint offset);
// immediate 8-bit value to accumulator (data-8 field)
static void decode_imm8(uint8_t * const image, uint offset);
// immediate 16-bit value to accumulator (from data-lo/data-hi fields without w)
static void decode_imm16(uint8_t * const image, uint offset);
// register (from reg and w fields)
static void decode_regw(uint8_t * const image, uint offset);
// memory (from mod, r/m and disp fields)
static void decode_mem16(uint8_t * const image, uint offset);
// byte registers (al-bh)
static void decode_reg_lo(uint8_t * const image, uint offset);
// word registers (ax-di)
static void decode_reg_hi(uint8_t * const image, uint offset);
// segment registers (es-ds)
static void decode_sr(uint8_t * const image, uint offset);
// segment registers (pop,push)
// NOTE: garbage solution
static void decode_sr1(uint8_t * const image, uint offset);
// direct address (addr fields)
static void decode_addr(uint8_t * const image, uint offset);
// short label address (from ip-inc-8 field)
static void decode_addr8(uint8_t * const image, uint offset);
// near proc/label address (from ip field)
static void decode_naddr(uint8_t * const image, uint offset);
// far proc/label address (from ip and cs fields)
static void decode_faddr(uint8_t * const image, uint offset);

// special case for 'int 3'
static void decode_3(uint8_t * const image, uint offset);
// special case for 'esc opcode, source'
static void decode_esc(uint8_t * const image, uint offset);
// special case for 'xchg ax, dx'
static void decode_ax(uint8_t * const image,  uint offset);
// special case for 'in al, dx', 'in ax, dx', 'out al, dx', 'out ax, dx'
static void decode_dx(uint8_t * const image, uint offset);
// special case for shift instructions (v bit: '1' or 'cl')
static void decode_v(uint8_t * const image, uint offset);
// special case for les, lds, lea (from reg field)
static void decode_reg_l(uint8_t * const image, uint offset);

static char *regs[2][16] =
{
	{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" }, // w = 0
	{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" }, // w = 1
};

static char *segregs[4] = { "es", "cs", "ss", "ds" };

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

struct inst_data
{
	// instruction mnemonic
	enum inst_type type;
	// function that decodes first parameter
	decode_fn      op1_dec;
	// function that decodes second parameter
	decode_fn      op2_dec;
	// (minimum) instruction size (in bytes)
	uint           size;
};

// NOTE: all instructions contain reg8/mem8 have 2 bytes size because
// displacement bytes can be ommitted, so actual size needs to be calculated for
// this instructions
struct inst_data inst_table[256] = {
	{ INST_TYPE_ADD,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_ADD,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_ADD,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_ADD,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_ADD,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_ADD,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_PUSH,   &decode_sr1,     NULL,            1 },
	{ INST_TYPE_POP,    &decode_sr1,     NULL,            1 },
	{ INST_TYPE_OR,     &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_OR,     &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_OR,     &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_OR,     &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_OR,     &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_OR,     &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_PUSH,   &decode_sr1,     NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_ADC,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_ADC,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_ADC,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_ADC,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_ADC,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_ADC,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_PUSH,   &decode_sr1,     NULL,            1 },
	{ INST_TYPE_POP,    &decode_sr1,     NULL,            1 },
	{ INST_TYPE_SBB,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_SBB,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_SBB,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_SBB,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_SBB,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_SBB,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_PUSH,   &decode_sr1,     NULL,            1 },
	{ INST_TYPE_POP,    &decode_sr1,     NULL,            1 },
	{ INST_TYPE_AND,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_AND,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_AND,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_AND,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_AND,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_AND,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_SGMNT,  NULL,            NULL,            1 },
	{ INST_TYPE_DAA,    NULL,            NULL,            1 },
	{ INST_TYPE_SUB,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_SUB,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_SUB,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_SUB,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_SUB,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_SUB,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_SGMNT,  NULL,            NULL,            1 },
	{ INST_TYPE_DAS,    NULL,            NULL,            1 },
	{ INST_TYPE_XOR,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_XOR,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_XOR,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_XOR,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_XOR,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_XOR,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_SGMNT,  NULL,            NULL,            1 },
	{ INST_TYPE_AAA,    NULL,            NULL,            1 },
	{ INST_TYPE_CMP,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_CMP,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_CMP,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_CMP,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_CMP,    &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_CMP,    &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_SGMNT,  NULL,            NULL,            1 },
	{ INST_TYPE_AAS,    NULL,            NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_INC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_DEC,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_PUSH,   &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_POP,    &decode_reg_hi,  NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_JO,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JNO,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JB,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JAE,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JE,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JNE,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JBE,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JA,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JS,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JNS,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JP,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JNP,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JL,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JGE,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JLE,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JG,     &decode_addr8,   NULL,            2 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_TEST,   &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_TEST,   &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_XCHG,   &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_XCHG,   &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_MOV,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_MOV,    &decode_reg_mem, &decode_regw,    2 },
	{ INST_TYPE_MOV,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_MOV,    &decode_regw,    &decode_reg_mem, 2 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_LEA,    &decode_regw,    &decode_mem16,   2 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_XCHG,   &decode_ax,      &decode_reg_hi,  1 },
	{ INST_TYPE_CBW,    NULL,            NULL,            1 },
	{ INST_TYPE_CWD,    NULL,            NULL,            1 },
	{ INST_TYPE_CALL,   &decode_faddr,   NULL,            5 },
	{ INST_TYPE_WAIT,   NULL,            NULL,            1 },
	{ INST_TYPE_PUSHF,  NULL,            NULL,            1 },
	{ INST_TYPE_POPF,   NULL,            NULL,            1 },
	{ INST_TYPE_SAHF,   NULL,            NULL,            1 },
	{ INST_TYPE_LAHF,   NULL,            NULL,            1 },
	{ INST_TYPE_MOV,    &decode_acc,     &decode_addr,    3 },
	{ INST_TYPE_MOV,    &decode_acc,     &decode_addr,    3 },
	{ INST_TYPE_MOV,    &decode_addr,    &decode_acc,     3 },
	{ INST_TYPE_MOV,    &decode_addr,    &decode_acc,     3 },
	{ INST_TYPE_MOVS,   NULL,            NULL,            1 },
	{ INST_TYPE_MOVS,   NULL,            NULL,            1 },
	{ INST_TYPE_CMPS,   NULL,            NULL,            1 },
	{ INST_TYPE_CMPS,   NULL,            NULL,            1 },
	{ INST_TYPE_TEST,   &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_TEST,   &decode_acc,     &decode_imm16,   3 },
	{ INST_TYPE_STDS,   NULL,            NULL,            1 },
	{ INST_TYPE_STDS,   NULL,            NULL,            1 },
	{ INST_TYPE_LODS,   NULL,            NULL,            1 },
	{ INST_TYPE_LODS,   NULL,            NULL,            1 },
	{ INST_TYPE_SCAS,   NULL,            NULL,            1 },
	{ INST_TYPE_SCAS,   NULL,            NULL,            1 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_lo,    &decode_imm8,  2 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_MOV,    &decode_reg_hi,   &decode_imm16,  3 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_RET,    &decode_imm16,   NULL,            3 },
	{ INST_TYPE_RET,    NULL,            NULL,            1 },
	{ INST_TYPE_LES,    &decode_reg_l,   &decode_mem16,   2 },
	{ INST_TYPE_LDS,    &decode_reg_l,   &decode_mem16,   2 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_RET,    &decode_imm16,   NULL,            3 },
	{ INST_TYPE_RET,    NULL,            NULL,            1 },
	{ INST_TYPE_INT,    &decode_3,       NULL,            1 },
	{ INST_TYPE_INT,    &decode_imm8,    NULL,            2 },
	{ INST_TYPE_INTO,   NULL,            NULL,            1 },
	{ INST_TYPE_IRET,   NULL,            NULL,            1 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	// NOTE: for some r eason these two instuctions take 2 bytes
	{ INST_TYPE_AAM,    NULL,            NULL,            2 },
	{ INST_TYPE_AAD,    NULL,            NULL,            2 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_XLAT,   NULL,            NULL,            1 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_ESC,    &decode_esc,     &decode_reg_mem, 2 },
	{ INST_TYPE_LOOPNE, &decode_addr8,   NULL,            2 },
	{ INST_TYPE_LOOPE,  &decode_addr8,   NULL,            2 },
	{ INST_TYPE_LOOP,   &decode_addr8,   NULL,            2 },
	{ INST_TYPE_JCXZ,   &decode_addr8,   NULL,            2 },
	{ INST_TYPE_IN,     &decode_acc,     &decode_imm8,    2 },
	{ INST_TYPE_IN,     &decode_acc,     &decode_imm8,    2 },
	// NOTE: for some reason I need to swap operands
	{ INST_TYPE_OUT,    &decode_imm8,    &decode_acc,     2 },
	{ INST_TYPE_OUT,    &decode_imm8,    &decode_acc,     2 },
	{ INST_TYPE_CALL,   &decode_naddr,   NULL,            3 },
	{ INST_TYPE_JMP,    &decode_naddr,   NULL,            3 },
	{ INST_TYPE_JMP,    &decode_faddr,   NULL,            5 },
	{ INST_TYPE_JMP,    &decode_addr8,   NULL,            2 },
	{ INST_TYPE_IN,     &decode_acc,     &decode_dx,      1 },
	{ INST_TYPE_IN,     &decode_acc,     &decode_dx,      1 },
	{ INST_TYPE_OUT,    &decode_dx,      &decode_acc,     1 },
	{ INST_TYPE_OUT,    &decode_dx,      &decode_acc,     1 },
	{ INST_TYPE_LOCK,   NULL,            NULL,            1 },
	{ INST_TYPE_UNK,    NULL,            NULL,            1 },
	{ INST_TYPE_REPNZ,  NULL,            NULL,            1 },
	{ INST_TYPE_REPZ,   NULL,            NULL,            1 },
	{ INST_TYPE_HLT,    NULL,            NULL,            1 },
	{ INST_TYPE_CMC,    NULL,            NULL,            1 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_CLC,    NULL,            NULL,            1 },
	{ INST_TYPE_STC,    NULL,            NULL,            1 },
	{ INST_TYPE_CLI,    NULL,            NULL,            1 },
	{ INST_TYPE_STI,    NULL,            NULL,            1 },
	{ INST_TYPE_CLD,    NULL,            NULL,            1 },
	{ INST_TYPE_STD,    NULL,            NULL,            1 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
	{ INST_TYPE_EXTD,   NULL,            NULL,            0 },
};

struct inst_data inst_table_extd[17][8] =
{
	{
		// | 1000 1000 | mod xxx r/m | disp-lo | disp-hi | data-8 |
		{ INST_TYPE_ADD,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_OR,   &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_ADC,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_SBB,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_AND,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_SUB,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_XOR,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_CMP,  &decode_reg_mem, &decode_imm,      3 },
	},
	{
		// | 1000 1001 | mod xxx r/m | disp-lo | disp-hi | data-lo | data-hi |
		{ INST_TYPE_ADD,  &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_OR,   &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_ADC,  &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_SBB,  &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_AND,  &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_SUB,  &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_XOR,  &decode_reg_mem, &decode_imm,      4 },
		{ INST_TYPE_CMP,  &decode_reg_mem, &decode_imm,      4 },
	},
	{
		// | 1000 1010 | mod xxx r/m | disp-lo | disp-hi | data-8 |
		{ INST_TYPE_ADD,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_UNK,  NULL,            NULL,             1 },
		{ INST_TYPE_ADC,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_SBB,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_UNK,  NULL,            NULL,             1 },
		{ INST_TYPE_SUB,  &decode_reg_mem, &decode_imm,      3 },
		{ INST_TYPE_UNK,  NULL,            NULL,             1 },
		{ INST_TYPE_CMP,  &decode_reg_mem, &decode_imm,      3 },
	},
	{
		// | 1000 1011 | mod xxx r/m | disp-lo | disp-hi | data-sx |
		{ INST_TYPE_ADD,  &decode_reg_mem, &decode_imm_sx,   3 },
		{ INST_TYPE_UNK,  NULL,            NULL,             1 },
		{ INST_TYPE_ADC,  &decode_reg_mem, &decode_imm_sx,   3 },
		{ INST_TYPE_SBB,  &decode_reg_mem, &decode_imm_sx,   3 },
		{ INST_TYPE_UNK,  NULL,            NULL,             1 },
		{ INST_TYPE_SUB,  &decode_reg_mem, &decode_imm_sx,   3 },
		{ INST_TYPE_UNK,  NULL,            NULL,             1 },
		{ INST_TYPE_CMP,  &decode_reg_mem, &decode_imm_sx,   3 },
	},
	{
		// | 1000 1100 | mod 0 sr r/m | disp-lo | disp-hi |
		{ INST_TYPE_MOV,  &decode_reg_mem, &decode_sr,       2 },
		{ INST_TYPE_MOV,  &decode_reg_mem, &decode_sr,       2 },
		{ INST_TYPE_MOV,  &decode_reg_mem, &decode_sr,       2 },
		{ INST_TYPE_MOV,  &decode_reg_mem, &decode_sr,       2 },
		// | 1000 1100 | mod 1 xx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
	{
		// | 1000 1110 | mod 0 sr r/m | disp-lo | disp-hi |
		{ INST_TYPE_MOV,  &decode_sr,      &decode_reg_mem,  2 },
		{ INST_TYPE_MOV,  &decode_sr,      &decode_reg_mem,  2 },
		{ INST_TYPE_MOV,  &decode_sr,      &decode_reg_mem,  2 },
		{ INST_TYPE_MOV,  &decode_sr,      &decode_reg_mem,  2 },
		// | 1000 1110 | mod 1 xx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
	{
		// | 1000 1111 | mod 000 r/m | disp-lo | disp-hi |
		{ INST_TYPE_POP,  &decode_reg_mem, NULL,             2 },
		// | 1000 1111 | mod xxx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
	{
		// | 1100 0110 | mod 000 r/m | disp-lo | disp-hi | data-8 |
		{ INST_TYPE_MOV,  &decode_reg_mem, &decode_imm,      3 },
		// | 1100 0110 | mod xxx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
	{
		// | 1100 0111 | mod 000 r/m | disp-lo | disp-hi | data-lo | data-hi |
		{ INST_TYPE_MOV,  &decode_reg_mem, &decode_imm,      4 },
		// | 1100 0111 | mod xxx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
	{
		// | 1101 0000 | mod xxx r/m | disp-lo | disp-hi |
		{ INST_TYPE_ROL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_ROR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_SAR,  &decode_reg_mem, &decode_v,        2 },
	},
	{
		// | 1101 0001 | mod xxx r/m | disp-lo | disp-hi |
		{ INST_TYPE_ROL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_ROR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_SAR,  &decode_reg_mem, &decode_v,        2 },
	},
	{
		// | 1101 0010 | mod xxx r/m | disp-lo | disp-hi |
		{ INST_TYPE_ROL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_ROR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_SAR,  &decode_reg_mem, &decode_v,        2 },
	},
	{
		// | 1101 0011 | mod xxx r/m | disp-lo | disp-hi |
		{ INST_TYPE_ROL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_ROR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_RCR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHL,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_SHR,  &decode_reg_mem, &decode_v,        2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_SAR,  &decode_reg_mem, &decode_v,        2 },
	},
	{
		// | 1111 0110 | mod 000 r/m | disp-lo | disl-hi | data-8 |
		{ INST_TYPE_TEST, &decode_reg_mem, &decode_imm,      3 },
		// | 1111 0110 | mod xxx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		// | 1111 0110 | mod xxx r/m | disp-lo | disl-hi |
		{ INST_TYPE_NOT,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_NEG,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_MUL,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_IMUL, &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_DIV,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_IDIV, &decode_reg_mem, NULL,             2 },
	},
	{
		// | 1111 0111 | mod 000 r/m | disp-lo | disl-hi | data-lo | data-hi |
		{ INST_TYPE_TEST, &decode_reg_mem, &decode_imm,      4 },
		// | 1111 0111 | mod xxx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		// | 1111 0111 | mod xxx r/m | disp-lo | disl-hi |
		{ INST_TYPE_NOT,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_NEG,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_MUL,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_IMUL, &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_DIV,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_IDIV, &decode_reg_mem, NULL,             2 },
	},
	{
		// | 1111 1110 | mod xxx r/m | disp-lo | disp-hi |
		{ INST_TYPE_INC,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_DEC,  &decode_reg_mem, NULL,             2 },
		// | 1111 1110 | mod xxx r/m |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
	{
		// | 1111 1111 | mod xxx r/m | disp-lo | disp-hi |
		{ INST_TYPE_INC,  &decode_mem16,   NULL,             2 },
		{ INST_TYPE_DEC,  &decode_mem16,   NULL,             2 },
		{ INST_TYPE_CALL, &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_CALL, &decode_mem16,   NULL,             2 },
		{ INST_TYPE_JMP,  &decode_reg_mem, NULL,             2 },
		{ INST_TYPE_JMP,  &decode_mem16,   NULL,             2 },
		{ INST_TYPE_PUSH, &decode_mem16,   NULL,             2 },
		// | 1111 1111 | mod 111 r/m | disp-lo | disp-hi |
		{ INST_TYPE_UNK,  NULL,            NULL,             2 },
	},
};

int main(int argc, char *argv[])
{
	int      rc = 0;
	size_t   nread;
	size_t   size;
	uint     i, offset, inst_count;
	int16_t  label_addr;

	FILE    *asm_file;
	uint8_t *image;

	struct bitmap bitmap_label;

	struct inst_data  inst;
	struct inst_data *insts;

	enum inst_type type;
	decode_fn op1, op2;
	uint8_t mod;

	char *sr_prefix = NULL;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s FILE\n\twhere FILE is a 8086 "
		        "assembly file to decode\n", argv[0]);
		rc = 1;
		goto exit_main;
	}

	asm_file = fopen(argv[1], "rb");
	if (asm_file == NULL) {
		perror("error: failed to open file");
		rc = 2;
		goto exit_main;
	}

	// determine file size
	fseek(asm_file, 0, SEEK_END);
	size = ftell(asm_file);
	rewind(asm_file);

	image = malloc(size);
	if (image == NULL) {
		perror("error: failed to allocate memory for a file image");
		rc = 3;
		goto fclose_file;
	}

	// copy file contents
	nread = fread(image, 1, size, asm_file);
	if (nread != size) {
		fprintf(stderr, "error: failed to read file contents");
		rc = 4;
		goto free_image;
	}

	// check if all opcodes in image are valid
	// and also count them
	offset = inst_count = 0;
	while (offset < size) {
		inst = get_inst_data(image, offset);

		if (inst.type == INST_TYPE_UNK) {
			fprintf(stderr, "error: unknown opcode is encountered: "
			        "0x%02X\n", image[offset]);
			rc = 5;
			goto free_image;
		}

		op1 = inst.op1_dec;
		op2 = inst.op2_dec;
		if (op1 == &decode_reg_mem || op2 == &decode_reg_mem ||
		    op1 == &decode_mem16   || op2 == &decode_mem16) {
			// add displacement size
			inst.size += calc_disp_size(image, offset);
		}

		offset += inst.size;
		inst_count++;
	}

	// label or not
	bitmap_init(&bitmap_label, size);

	insts = malloc(inst_count * sizeof(*insts));
	if (insts == NULL) {
		perror("error: failed to allocate memory for data neccesary to "
		       "decode instructions");
		rc = 6;
		goto free_bitmap;
	}

	i = offset = 0;
	while (i < inst_count) {
		insts[i] = get_inst_data(image, offset);

		op1 = insts[i].op1_dec;
		op2 = insts[i].op2_dec;

		// add displacement size if necessary
		if (op1 == &decode_reg_mem || op2 == &decode_reg_mem ||
		    op1 == &decode_mem16   || op2 == &decode_mem16) {
			insts[i].size += calc_disp_size(image, offset);
		}

		// get label address if it's a jump instruction
		label_addr = get_label_addr(insts[i].type, image, offset);
#if 0
		if (label_addr > (int)size) {
			fprintf(stderr, "invalid label address: %X\n",
			        label_addr);
			rc = 7;
			goto free_insts;
		}
#endif

		// set bits where label should be placed
		if (label_addr >= 0) {
			bitmap_set_bit(&bitmap_label, label_addr);
		}

		offset += insts[i].size;
		i++;
	}

	// generate output
	printf("; %s\n", argv[1]);
	printf("\nbits 16\n\n");

	i = offset = 0;
	while (i < inst_count) {
		// handle labels
		if (bitmap_get_bit(&bitmap_label, offset) == 1) {
			printf("label_%u:\n", offset);
		}

		// mnemonic
		printf("%s", gen_inst_name(insts[i].type, image, offset));

		type = insts[i].type;
		op1  = insts[i].op1_dec;
		op2  = insts[i].op2_dec;
		mod  = FIELD_MOD(image[offset + 1]);

		// print byte size if:
		// 1. the only operand is memory
		// 2. first operand is memory and second is not register
		// 3. operand not one of the jump instructions
		if (((op1 == &decode_mem16 && op2 == NULL) ||
		    (op1 == &decode_reg_mem && mod != 0b11 &&
		     op2 != &decode_regw)) &&
		    type != INST_TYPE_CALL &&
		    type != INST_TYPE_RET &&
		    type != INST_TYPE_JMP) {
			if (FIELD_W(image[offset])) {
				printf(" word");
			} else {
				printf(" byte");
			}
		}

		if (op1 != NULL) {
			printf(" ");
			// segment register prefix
			if ((op1 == &decode_reg_mem || op1 == &decode_mem16) &&
			     sr_prefix) {
				printf("%s:", sr_prefix);
				sr_prefix = NULL;
			}

			op1(image, offset);
		}

		if (op2 != NULL) {
			printf(", ");
			if ((op2 == &decode_reg_mem || op2 == &decode_mem16) &&
			     sr_prefix) {
				printf("%s:", sr_prefix);
				sr_prefix = NULL;
			}

			op2(image, offset);
		}

		// handle prefixes
		switch (insts[i].type) {
		case INST_TYPE_REPZ:
		case INST_TYPE_REPNZ:
		case INST_TYPE_LOCK:
			printf(" ");
			break;
		case INST_TYPE_SGMNT:
			sr_prefix = segregs[FIELD_SR(image[offset])];
			break;
		default:
			sr_prefix = NULL;
			printf("\n");
		}

		offset += insts[i].size;
		i++;
	}

#if 0
free_insts:
#endif
	free(insts);
free_bitmap:
	bitmap_free(&bitmap_label);
free_image:
	free(image);
fclose_file:
	fclose(asm_file);
exit_main:
	return rc;
}

struct inst_data get_inst_data(uint8_t * const image, uint offset)
{
	uint8_t extd_op = 0, i = 0;
	struct inst_data data;

	data = inst_table[image[offset]];

	if (data.type == INST_TYPE_EXTD) {
		// NOTE: maybe there is a way to do it better, but I'm too tired
		switch (image[offset]) {
		case 0x80: i = 0;  break;
		case 0x81: i = 1;  break;
		case 0x82: i = 2;  break;
		case 0x83: i = 3;  break;
		case 0x8C: i = 4;  break;
		case 0x8E: i = 5;  break;
		case 0x8F: i = 6;  break;
		case 0xC6: i = 7;  break;
		case 0xC7: i = 8;  break;
		case 0xD0: i = 9;  break;
		case 0xD1: i = 10; break;
		case 0xD2: i = 11; break;
		case 0xD3: i = 12; break;
		case 0xF6: i = 13; break;
		case 0xF7: i = 14; break;
		case 0xFE: i = 15; break;
		case 0xFF: i = 16; break;
		default:
			assert(image[offset] != image[offset]);
		}

		extd_op = (image[offset + 1] >> 3) & 0b111;
		data    = inst_table_extd[i][extd_op];
	}

	return data;
}

const char *gen_inst_name(enum inst_type type, uint8_t * const image,
                          uint offset)
{
	const char * mnem = "";
	switch (type) {
		case INST_TYPE_MOV:    mnem = "mov";    break;
		case INST_TYPE_PUSH:   mnem = "push";   break;
		case INST_TYPE_POP:    mnem = "pop";    break;
		case INST_TYPE_XCHG:   mnem = "xchg";   break;
		case INST_TYPE_IN:     mnem = "in";     break;
		case INST_TYPE_OUT:    mnem = "out";    break;
		case INST_TYPE_XLAT:   mnem = "xlat";   break;
		case INST_TYPE_LEA:    mnem = "lea";    break;
		case INST_TYPE_LDS:    mnem = "lds";    break;
		case INST_TYPE_LES:    mnem = "les";    break;
		case INST_TYPE_LAHF:   mnem = "lahf";   break;
		case INST_TYPE_SAHF:   mnem = "sahf";   break;
		case INST_TYPE_PUSHF:  mnem = "pushf";  break;
		case INST_TYPE_POPF:   mnem = "popf";   break;
		case INST_TYPE_ADD:    mnem = "add";    break;
		case INST_TYPE_ADC:    mnem = "adc";    break;
		case INST_TYPE_INC:    mnem = "inc";    break;
		case INST_TYPE_AAA:    mnem = "aaa";    break;
		case INST_TYPE_DAA:    mnem = "daa";    break;
		case INST_TYPE_SUB:    mnem = "sub";    break;
		case INST_TYPE_SBB:    mnem = "sbb";    break;
		case INST_TYPE_DEC:    mnem = "dec";    break;
		case INST_TYPE_NEG:    mnem = "neg";    break;
		case INST_TYPE_CMP:    mnem = "cmp";    break;
		case INST_TYPE_AAS:    mnem = "aas";    break;
		case INST_TYPE_DAS:    mnem = "das";    break;
		case INST_TYPE_MUL:    mnem = "mul";    break;
		case INST_TYPE_IMUL:   mnem = "imul";   break;
		case INST_TYPE_AAM:    mnem = "aam";    break;
		case INST_TYPE_DIV:    mnem = "div";    break;
		case INST_TYPE_IDIV:   mnem = "idiv";   break;
		case INST_TYPE_AAD:    mnem = "aad";    break;
		case INST_TYPE_CBW:    mnem = "cbw";    break;
		case INST_TYPE_CWD:    mnem = "cwd";    break;
		case INST_TYPE_NOT:    mnem = "not";    break;
		case INST_TYPE_SHL:    mnem = "shl";    break;
		case INST_TYPE_SHR:    mnem = "shr";    break;
		case INST_TYPE_SAR:    mnem = "sar";    break;
		case INST_TYPE_ROL:    mnem = "rol";    break;
		case INST_TYPE_ROR:    mnem = "ror";    break;
		case INST_TYPE_RCL:    mnem = "rcl";    break;
		case INST_TYPE_RCR:    mnem = "rcr";    break;
		case INST_TYPE_AND:    mnem = "and";    break;
		case INST_TYPE_TEST:   mnem = "test";   break;
		case INST_TYPE_OR:     mnem = "or";     break;
		case INST_TYPE_XOR:    mnem = "xor";    break;
		// NOTE: changed to 'repne' and 'rep'
		case INST_TYPE_REPNZ:  mnem = "repne";  break;
		case INST_TYPE_REPZ:   mnem = "rep";    break;
		case INST_TYPE_MOVS:
			mnem = "movsb";
			if (FIELD_W(image[offset])) {
				mnem = "movsw";
			}

			break;
		case INST_TYPE_CMPS:
			mnem = "cmpsb";
			if (FIELD_W(image[offset])) {
				mnem = "cmpsw";
			}

			break;
		case INST_TYPE_SCAS:
			mnem = "scasb";
			if (FIELD_W(image[offset])) {
				mnem = "scasw";
			}

			break;
		case INST_TYPE_LODS:
			mnem = "lodsb";
			if (FIELD_W(image[offset])) {
				mnem = "lodsw";
			}

			break;
		case INST_TYPE_STDS:   mnem = "stds";   break;
		case INST_TYPE_CALL:   mnem = "call";   break;
		case INST_TYPE_JMP:    mnem = "jmp";    break;
		case INST_TYPE_RET:    mnem = "ret";    break;
		case INST_TYPE_JE:     mnem = "je";     break;
		case INST_TYPE_JL:     mnem = "jl";     break;
		case INST_TYPE_JLE:    mnem = "jle";    break;
		case INST_TYPE_JB:     mnem = "jb";     break;
		case INST_TYPE_JBE:    mnem = "jbe";    break;
		case INST_TYPE_JP:     mnem = "jp";     break;
		case INST_TYPE_JO:     mnem = "jo";     break;
		case INST_TYPE_JS:     mnem = "js";     break;
		case INST_TYPE_JNE:    mnem = "jne";    break;
		case INST_TYPE_JGE:    mnem = "jge";    break;
		case INST_TYPE_JG:     mnem = "jg";     break;
		case INST_TYPE_JAE:    mnem = "jae";    break;
		case INST_TYPE_JA:     mnem = "ja";     break;
		case INST_TYPE_JNP:    mnem = "jnp";    break;
		case INST_TYPE_JNO:    mnem = "jno";    break;
		case INST_TYPE_JNS:    mnem = "jns";    break;
		case INST_TYPE_LOOP:   mnem = "loop";   break;
		case INST_TYPE_LOOPE:  mnem = "loope";  break;
		case INST_TYPE_LOOPNE: mnem = "loopne"; break;
		case INST_TYPE_JCXZ:   mnem = "jcxz";   break;
		case INST_TYPE_INT:    mnem = "int";    break;
		case INST_TYPE_INTO:   mnem = "into";   break;
		case INST_TYPE_IRET:   mnem = "iret";   break;
		case INST_TYPE_CLC:    mnem = "clc";    break;
		case INST_TYPE_CMC:    mnem = "cmc";    break;
		case INST_TYPE_STC:    mnem = "stc";    break;
		case INST_TYPE_CLD:    mnem = "cld";    break;
		case INST_TYPE_STD:    mnem = "std";    break;
		case INST_TYPE_CLI:    mnem = "cli";    break;
		case INST_TYPE_STI:    mnem = "sti";    break;
		case INST_TYPE_HLT:    mnem = "hlt";    break;
		case INST_TYPE_WAIT:   mnem = "wait";   break;
		case INST_TYPE_ESC:    mnem = "esc";    break;
		case INST_TYPE_LOCK:   mnem = "lock";   break;
		case INST_TYPE_SGMNT:  mnem = "";       break;
		default:
			assert(type != type);
	}

	return mnem;
}

int get_label_addr(enum inst_type type, uint8_t * const image, uint offset)
{
	int      label_addr = -1;
	uint8_t  tmp8;
	uint16_t tmp16;

	// if this is a jump instruction, determine label address
	switch (type) {
	case INST_TYPE_JO:
	case INST_TYPE_JNO:
	case INST_TYPE_JB:
	case INST_TYPE_JAE:
	case INST_TYPE_JE:
	case INST_TYPE_JNE:
	case INST_TYPE_JBE:
	case INST_TYPE_JA:
	case INST_TYPE_JS:
	case INST_TYPE_JNS:
	case INST_TYPE_JP:
	case INST_TYPE_JNP:
	case INST_TYPE_JL:
	case INST_TYPE_JGE:
	case INST_TYPE_JLE:
	case INST_TYPE_JG:
	case INST_TYPE_LOOPNE:
	case INST_TYPE_LOOPE:
	case INST_TYPE_LOOP:
	case INST_TYPE_JCXZ:
		// these instructions have only ip-inc8 field
		// which is next byte
		// read as signed 8-bit integer
		tmp8 = image[offset + 1];
		label_addr = offset + 2;
		label_addr += *((int8_t *)&tmp8);
		break;
	case INST_TYPE_CALL:
	case INST_TYPE_JMP:
		// handle jmp instruction, which can have variable
		// length address

		// direct within segment (near-label)
		if (((image[offset] >> 1) & 0b111) == 0b100) {
			tmp16 = (image[offset + 2] << 8) | image[offset + 1];
			label_addr = offset + 3 + *((int16_t *)&tmp16);
			break;
		}

		// direct intersegment (far-label)
		if (((image[offset] >> 1) & 0b111) == 0b101) {
			// direct within segment-short (short-label)
			if (type == INST_TYPE_JMP && image[offset] & 1) {
				tmp8 = image[offset + 1];
				label_addr = offset + 2 + *((int8_t *)&tmp8);
				break;
			}
		}

		break;
	default:
		label_addr = -1;
	}

	return label_addr;
}

uint calc_disp_size(uint8_t * const image, uint offset)
{
	uint    disp_size = 0;
	uint8_t mod, r_m;
	uint8_t *inst = image + offset;

	mod = FIELD_MOD(inst[1]);
	r_m = FIELD_RM(inst[1]);

	if (mod != 0b11) {
		// direct address
		if (mod == 0b00 && r_m == 0b110) {
			disp_size = 2;
		}
		// 8-bit displacement
		if (mod == 0b01) {
			disp_size = 1;
		}
		// 16-bit displacement
		if (mod == 0b10) {
			disp_size = 2;
		}
	}

	return disp_size;
}

void decode_reg_mem(uint8_t * const image, uint offset)
{
	int   len;
	char  ea_str[64];
	char *op;

	uint8_t  w, mod, r_m;
	uint8_t *inst = image + offset;
	int16_t disp;

	w   = FIELD_W(inst[0]);
	mod = FIELD_MOD(inst[1]);
	r_m = FIELD_RM(inst[1]);

	op = regs[w][r_m];

	if (mod != 0b11) {
		disp = (inst[3] << 8) | inst[2];

		if (mod == 0b00 && r_m == 0b110) {
			len = snprintf(ea_str, sizeof(ea_str), "[%u]",
			               disp & 0xFFFF);
		} else {
			// [ ea_base + d8 ]
			if (mod == 0b01) {
				// only low byte
				disp &= 0x00FF;
				// if sign bit is set then sign-extend
				if (disp & 0x80) {
					disp |= 0xFF00;
				}

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
	}

	printf("%s", op);
}

void decode_acc(uint8_t * const image, uint offset)
{
	if (FIELD_W(image[offset])) {
		printf("ax");
	} else {
		printf("al");
	}
}

void decode_imm(uint8_t * const image, uint offset)
{
	uint8_t w, disp_size;
	uint8_t *inst = image + offset;
	uint16_t imm;

	w = FIELD_W(inst[0]);

	// calculate displacement size to determinte immediate value position
	disp_size = calc_disp_size(image, offset);

	imm = (inst[3 + disp_size] << 8) * w | inst[2 + disp_size];

	printf("%u", imm);
}

void decode_imm_sx(uint8_t * const image, uint offset)
{
	uint8_t disp_size;
	uint8_t *inst = image + offset;
	uint16_t imm;

	// calculate displacement size to determinte immediate value position
	disp_size = calc_disp_size(image, offset);

	imm = inst[2 + disp_size];

	// if sign bit is set then sign-extend
	if (imm & 0x80) {
		imm |= 0xFF00;
	}

	printf("%u", imm);
}

void decode_imm8(uint8_t * const image, uint offset)
{
	printf("%u", image[offset + 1]);
}

void decode_imm16(uint8_t * const image, uint offset)
{
	uint8_t *inst = image + offset;
	uint16_t imm;

	imm = (inst[2] << 8) | inst[1];
	printf("%u", imm);
}

void decode_regw(uint8_t * const image, uint offset)
{
	uint8_t w, reg;
	uint8_t *inst = image + offset;

	w   = FIELD_W(inst[0]);
	reg = FIELD_REG(inst[1]);

	printf("%s", regs[w][reg]);
}

void decode_mem16(uint8_t * const image, uint offset)
{
	uint8_t *inst = image + offset;
	uint8_t mod = FIELD_MOD(inst[1]);

	assert(mod != 0b11); // memory only

	decode_reg_mem(image, offset);
}

void decode_reg_lo(uint8_t * const image, uint offset)
{
	uint8_t reg;
	reg = FIELD_REG2(image[offset]);

	printf("%s", regs[0][reg]);
}

void decode_reg_hi(uint8_t * const image, uint offset)
{
	uint8_t reg;
	reg = FIELD_REG2(image[offset]);

	printf("%s", regs[1][reg]);
}

void decode_sr(uint8_t * const image, uint offset)
{
	uint8_t sr;
	uint8_t *inst = image + offset;

	sr = FIELD_SR(inst[1]);

	printf("%s", segregs[sr]);
}

void decode_sr1(uint8_t * const image, uint offset)
{
	decode_sr(image, offset - 1);
}

void decode_addr(uint8_t * const image, uint offset)
{
	uint8_t *inst = image + offset;
	uint16_t addr;

	addr = (inst[2] << 8) | inst[1];
	printf("[%u]", addr);
}

void decode_addr8(uint8_t * const image, uint offset)
{
	int addr = offset + 2;
	addr += *((int8_t *)(image + offset + 1));
	printf("label_%u", addr);
}

// TODO: fix this
void decode_naddr(uint8_t * const image, uint offset)
{
	uint16_t tmp;
	uint8_t *inst = image + offset;
	int16_t  addr = offset;

	tmp = (inst[2] << 8) | inst[1];

	addr += *((int16_t *)&tmp);
	printf("%d", addr);
}

void decode_faddr(uint8_t * const image, uint offset)
{
	uint8_t *inst = image + offset;
	uint16_t ip_addr, cs_addr;

	ip_addr = (inst[2] << 8) | inst[1];
	cs_addr = (inst[4] << 8) | inst[3];

	printf("%u:%u", cs_addr, ip_addr);
}

void decode_3(uint8_t * const image, uint offset)
{
	(void)image; (void)offset;

	printf("3");
}

void decode_esc(uint8_t * const image, uint offset)
{
	(void)image; (void)offset;
}

void decode_ax(uint8_t * const image,  uint offset)
{
	(void)image; (void)offset;
	printf("ax");
}

void decode_dx(uint8_t * const image, uint offset)
{
	(void)image; (void)offset;
	printf("dx");
}

void decode_v(uint8_t * const image, uint offset)
{
	uint8_t *inst = image + offset;
	uint8_t v;

	v = FIELD_V(inst[0]);
	
	if (v) {
		printf("cl");
	} else {
		printf("1");
	}
}

void decode_reg_l(uint8_t * const image, uint offset)
{
	uint8_t reg;
	uint8_t *inst = image + offset;

	reg = FIELD_REG(inst[1]);
	printf("%s", regs[1][reg]);
}
