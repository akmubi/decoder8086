#if !defined INST_H
#define INST_H

#include "common.h"

#define FLAG_W  (0b1  << 0)
#define FLAG_D  (0b1  << 1)
#define FLAG_S  (0b1  << 2)
#define FLAG_V  (0b1  << 3)
#define FLAG_ES (0b00 << 4)
#define FLAG_CS (0b01 << 4)
#define FLAG_SS (0b10 << 4)
#define FLAG_DS (0b11 << 4)
#define FLAG_MO (0b1  << 6) // memory only
#define FLAG_LB (0b1  << 7) // has label

#define SR_OP(flags) (((flags) >> 4) & 0b11)

// implicit prefixes
#define PFX_WIDE     (0b1  << 0)
#define PFX_FAR      (0b1  << 1)
// explicit prefixes
#define PFX_LOCK     (0b1  << 2)
#define PFX_SGMNT    (0b1  << 3)
#define PFX_SGMNT_ES (0b00 << 4)
#define PFX_SGMNT_CS (0b01 << 4)
#define PFX_SGMNT_SS (0b10 << 4)
#define PFX_SGMNT_DS (0b11 << 4)
#define PFX_REP      (0b1  << 6)
#define PFX_REPNE    (0b1  << 7)

#define SGMNT_OP(prefixes) (((prefixes) >> 4) & 0b11)

#define FIELD_MOD(byte) (((byte) >>  0) & 0b11)
#define FIELD_SR(byte)  (((byte) >>  2) & 0b11)
#define FIELD_RM(byte)  (((byte) >>  4) & 0b111)
#define FIELD_REG(byte) (((byte) >>  7) & 0b111)
#define FIELD_ESC(byte) (((byte) >> 10) & 0b111111)

enum inst_format
{
	INST_FMT_NONE,

	// [mod ... r/m] [disp-lo] [disp-hi]
	INST_FMT_RM,
	// [mod ... r/m] [disp-lo] [disp-hi] (store 1/cl)
	INST_FMT_RM_V,
	// [mod 0 sr r/m] [disp-lo] [disp-hi]
	INST_FMT_RM_SR,
	// [mod reg r/m] [disp-lo] [disp-hi]
	INST_FMT_RM_REG,
	// [mod ... r/m] [disp-lo] [disp-hi] [data]
	INST_FMT_RM_IMM,
	// [... xxx] [mod yyy r/m] [disp-lo] [disp-hi]
	INST_FMT_RM_ESC,

	INST_FMT_ACC_DX,
	// [data-8]
	INST_FMT_ACC_IMM8,
	// [data-lo] [data-hi]
	INST_FMT_ACC_IMM,
	// [... reg]
	INST_FMT_ACC_REG,
	// [addr-lo] [addr-hi]
	INST_FMT_ACC_MEM,

	// [... reg]
	INST_FMT_REG,
	// [...reg] [data-hi] [data-lo]
	INST_FMT_REG_IMM,

	// [... sr ...]
	INST_FMT_SR,

	// [data-lo] [data-hi]
	INST_FMT_IMM,

	// [ip-inc-8]
	INST_FMT_JMP_SHORT,
	// [ip-inc-lo] [ip-inc-hi]
	INST_FMT_JMP_NEAR,
	// [ip-lo] [ip-hi] [cs-lo] [cs-hi]
	INST_FMT_JMP_FAR,
 };

enum inst_type
{
	INST_UNK = 0,

	INST_AAA,
	INST_AAD,
	INST_AAM,
	INST_AAS,
	INST_ADC,
	INST_ADD,
	INST_AND,
	INST_CALL,
	INST_CALLF,
	INST_CBW,
	INST_CLC,
	INST_CLD,
	INST_CLI,
	INST_CMC,
	INST_CMP,
	INST_CMPSB,
	INST_CMPSW,
	INST_CWD,
	INST_DAA,
	INST_DAS,
	INST_DEC,
	INST_DIV,
	INST_ESC,
	INST_HLT,
	INST_IDIV,
	INST_IMUL,
	INST_IN,
	INST_INC,
	INST_INT,
	INST_INT3,
	INST_INTO,
	INST_IRET,
	INST_JA,
	INST_JAE,
	INST_JB,
	INST_JBE,
	INST_JCXZ,
	INST_JE,
	INST_JG,
	INST_JGE,
	INST_JL,
	INST_JLE,
	INST_JMP,
	INST_JMPF,
	INST_JNE,
	INST_JNO,
	INST_JNS,
	INST_JO,
	INST_JP,
	INST_JPO,
	INST_JS,
	INST_LAHF,
	INST_LDS,
	INST_LEA,
	INST_LES,
	INST_LOCK,
	INST_LODSB,
	INST_LODSW,
	INST_LOOP,
	INST_LOOPZ,
	INST_LOOPNZ,
	INST_MOV,
	INST_MOVSB,
	INST_MOVSW,
	INST_MUL,
	INST_NEG,
	INST_NOP,
	INST_NOT,
	INST_OR,
	INST_OUT,
	INST_POP,
	INST_POPF,
	INST_PUSH,
	INST_PUSHF,
	INST_RCL,
	INST_RCR,
	INST_REP,
	INST_REPNE,
	INST_RET,
	INST_RETF,
	INST_ROL,
	INST_ROR,
	INST_SAHF,
	INST_SAR,
	INST_SBB,
	INST_SCASB,
	INST_SCASW,
	INST_SGMNT,
	INST_SHL,
	INST_SHR,
	INST_STC,
	INST_STD,
	INST_STOSB,
	INST_STOSW,
	INST_STI,
	INST_SUB,
	INST_TEST,
	INST_WAIT,
	INST_XCHG,
	INST_XLAT,
	INST_XOR,

	INST_EXTD,
};

struct inst_data
{
	enum inst_type   type;
	enum inst_format fmt;
	uint8            flags;    // w, d, s, v, sr, mo, lb
	uint8            prefixes; // lock, sgmnt, rep, repne
	uint8            size;
};

struct inst
{
	struct inst_data base;
	uint16 disp;
	uint16 data;     // addr, imm
	uint16 data_ext; // if instruction size is 6 bytes
	uint16 fields;   // mod, reg, r/m, sr, esc
	uint   offset;   // location in image
};

// Calculates target offset of a jmp instruction (call, jmp, jne etc). Returns
// offset on success and negative value if error occurred.
extern int get_jmp_offset(struct inst *inst);

// extracts instruction data from 'image' at given 'offset'. Returns 0 on
// success and negative value if error occurred.
extern int get_inst(struct inst *inst, uint8 * const image, uint size,
                    uint offset);

// Extracts instructions from 'image' and writes 'count' instructions into
// 'insts' array. If 'insts' is NULL, function returns instruction count. If
// invalid instruction is encountered or error occurred negative value is
// returned. Returns 0 on success.
extern int inst_scan_image(struct inst * const insts, uint count,
                           uint8 * const image, uint size);

#endif /* INST_H */
