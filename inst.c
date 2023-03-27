#include <assert.h>

#include "inst.h"
#include "bitmap.h"

static uint calc_disp_size(uint8 * const image, uint offset);

struct inst_data inst_table[256] =
{
	{ INST_ADD,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x00
	{ INST_ADD,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x01
	{ INST_ADD,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x02
	{ INST_ADD,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x03
	{ INST_ADD,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x04
	{ INST_ADD,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x05
	{ INST_PUSH,   INST_FMT_SR,        FLAG_ES,               0, 1 }, // 0x06
	{ INST_POP,    INST_FMT_SR,        FLAG_ES,               0, 1 }, // 0x07
	{ INST_OR,     INST_FMT_RM_REG,    0,                     0, 2 }, // 0x08
	{ INST_OR,     INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x09
	{ INST_OR,     INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x0A
	{ INST_OR,     INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x0B
	{ INST_OR,     INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x0C
	{ INST_OR,     INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x0D
	{ INST_PUSH,   INST_FMT_SR,        FLAG_CS,               0, 1 }, // 0x0E
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x0F
	{ INST_ADC,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x10
	{ INST_ADC,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x11
	{ INST_ADC,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x12
	{ INST_ADC,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x13
	{ INST_ADC,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x14
	{ INST_ADC,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x15
	{ INST_PUSH,   INST_FMT_SR,        FLAG_SS,               0, 1 }, // 0x16
	{ INST_POP,    INST_FMT_SR,        FLAG_SS,               0, 1 }, // 0x17
	{ INST_SBB,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x18
	{ INST_SBB,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x19
	{ INST_SBB,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x1A
	{ INST_SBB,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x1B
	{ INST_SBB,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x1C
	{ INST_SBB,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x1D
	{ INST_PUSH,   INST_FMT_SR,        FLAG_DS,               0, 1 }, // 0x1E
	{ INST_POP,    INST_FMT_SR,        FLAG_DS,               0, 1 }, // 0x1F
	{ INST_AND,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x20
	{ INST_AND,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x21
	{ INST_AND,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x22
	{ INST_AND,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x23
	{ INST_AND,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x24
	{ INST_AND,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x25
	{ INST_SGMNT,  INST_FMT_SR,        FLAG_ES,               0, 1 }, // 0x26
	{ INST_DAA,    INST_FMT_NONE,      0,                     0, 1 }, // 0x27
	{ INST_SBB,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x28
	{ INST_SBB,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x29
	{ INST_SBB,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x2A
	{ INST_SBB,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x2B
	{ INST_SBB,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x2C
	{ INST_SBB,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x2D
	{ INST_SGMNT,  INST_FMT_SR,        FLAG_CS,               0, 1 }, // 0x2E
	{ INST_DAS,    INST_FMT_NONE,      0,                     0, 1 }, // 0x2F
	{ INST_XOR,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x30
	{ INST_XOR,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x31
	{ INST_XOR,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x32
	{ INST_XOR,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x33
	{ INST_XOR,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x34
	{ INST_XOR,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x35
	{ INST_SGMNT,  INST_FMT_SR,        FLAG_SS,               0, 1 }, // 0x36
	{ INST_AAA,    INST_FMT_NONE,      0,                     0, 1 }, // 0x37
	{ INST_CMP,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x38
	{ INST_CMP,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x39
	{ INST_CMP,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x3A
	{ INST_CMP,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x3B
	{ INST_CMP,    INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0x3C
	{ INST_CMP,    INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0x3D
	{ INST_SGMNT,  INST_FMT_SR,        FLAG_DS,               0, 1 }, // 0x3E
	{ INST_AAS,    INST_FMT_NONE,      0,                     0, 1 }, // 0x3F
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x40
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x41
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x42
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x43
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x44
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x45
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x46
	{ INST_INC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x47
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x48
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x49
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x4A
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x4B
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x4C
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x4D
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x4E
	{ INST_DEC,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x4F
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x50
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x51
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x52
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x53
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x54
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x55
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x56
	{ INST_PUSH,   INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x57
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x58
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x59
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x5A
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x5B
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x5C
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x5D
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x5E
	{ INST_POP,    INST_FMT_REG,       FLAG_W,                0, 1 }, // 0x5F
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x60
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x61
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x62
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x63
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x64
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x65
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x66
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x67
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x68
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x69
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x6A
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x6B
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x6C
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x6D
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x6E
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0x6F
	{ INST_JO,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x70
	{ INST_JNO,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x71
	{ INST_JB,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x72
	{ INST_JAE,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x73
	{ INST_JE,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x74
	{ INST_JNE,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x75
	{ INST_JBE,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x76
	{ INST_JA,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x77
	{ INST_JS,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x78
	{ INST_JNS,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x79
	{ INST_JP,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x7A
	{ INST_JPO,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x7B
	{ INST_JL,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x7C
	{ INST_JGE,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x7D
	{ INST_JLE,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x7E
	{ INST_JG,     INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0x7F
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x80
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x81
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x82
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x83
	{ INST_TEST,   INST_FMT_RM_REG,    0,                     0, 2 }, // 0x84
	{ INST_TEST,   INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x85
	{ INST_TEST,   INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x86
	{ INST_TEST,   INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x87
	{ INST_MOV,    INST_FMT_RM_REG,    0,                     0, 2 }, // 0x88
	{ INST_MOV,    INST_FMT_RM_REG,    FLAG_W,                0, 2 }, // 0x89
	{ INST_MOV,    INST_FMT_RM_REG,    FLAG_D,                0, 2 }, // 0x8A
	{ INST_MOV,    INST_FMT_RM_REG,    FLAG_D|FLAG_W,         0, 2 }, // 0x8B
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x8C
	{ INST_LEA,    INST_FMT_RM_REG,    FLAG_D|FLAG_W|FLAG_MO, 0, 2 }, // 0x8D
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x8E
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0x8F
	{ INST_NOP,    INST_FMT_NONE,      FLAG_W,                0, 1 }, // 0x90
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x91
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x92
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x93
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x94
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x95
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x96
	{ INST_XCHG,   INST_FMT_ACC_REG,   FLAG_W,                0, 1 }, // 0x97
	{ INST_CBW,    INST_FMT_NONE,      0,                     0, 1 }, // 0x98
	{ INST_CWD,    INST_FMT_NONE,      0,                     0, 1 }, // 0x99
	{ INST_CALL,   INST_FMT_JMP_FAR,   0,                     0, 5 }, // 0x9A
	{ INST_WAIT,   INST_FMT_NONE,      0,                     0, 1 }, // 0x9B
	{ INST_PUSHF,  INST_FMT_NONE,      0,                     0, 1 }, // 0x9C
	{ INST_POPF,   INST_FMT_NONE,      0,                     0, 1 }, // 0x9D
	{ INST_SAHF,   INST_FMT_NONE,      0,                     0, 1 }, // 0x9E
	{ INST_LAHF,   INST_FMT_NONE,      0,                     0, 1 }, // 0x9F
	{ INST_MOV,    INST_FMT_ACC_MEM,   FLAG_MO,               0, 3 }, // 0xA0
	{ INST_MOV,    INST_FMT_ACC_MEM,   FLAG_W|FLAG_MO,        0, 3 }, // 0xA1
	{ INST_MOV,    INST_FMT_ACC_MEM,   FLAG_D|FLAG_MO,        0, 3 }, // 0xA2
	{ INST_MOV,    INST_FMT_ACC_MEM,   FLAG_D|FLAG_W|FLAG_MO, 0, 3 }, // 0xA3
	{ INST_MOVSB,  INST_FMT_NONE,      0,                     0, 1 }, // 0xA4
	{ INST_MOVSW,  INST_FMT_NONE,      FLAG_W,                0, 1 }, // 0xA5
	{ INST_CMPSB,  INST_FMT_NONE,      0,                     0, 1 }, // 0xA6
	{ INST_CMPSW,  INST_FMT_NONE,      FLAG_W,                0, 1 }, // 0xA7
	{ INST_TEST,   INST_FMT_ACC_IMM,   0,                     0, 2 }, // 0xA8
	{ INST_TEST,   INST_FMT_ACC_IMM,   FLAG_W,                0, 3 }, // 0xA9
	{ INST_STDSB,  INST_FMT_NONE,      0,                     0, 1 }, // 0xAA
	{ INST_STDSW,  INST_FMT_NONE,      0,                     0, 1 }, // 0xAB
	{ INST_LODSB,  INST_FMT_NONE,      0,                     0, 1 }, // 0xAC
	{ INST_LODSW,  INST_FMT_NONE,      0,                     0, 1 }, // 0xAD
	{ INST_SCASB,  INST_FMT_NONE,      0,                     0, 1 }, // 0xAE
	{ INST_SCASW,  INST_FMT_NONE,      0,                     0, 1 }, // 0xAF
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB0
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB1
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB2
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB3
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB4
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB5
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB6
	{ INST_MOV,    INST_FMT_REG_IMM,   0,                     0, 2 }, // 0xB7
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xB8
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xB9
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xBA
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xBB
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xBC
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xBD
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xBE
	{ INST_MOV,    INST_FMT_REG_IMM,   FLAG_W,                0, 3 }, // 0xBF
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0xC0
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0xC1
	{ INST_RET,    INST_FMT_IMM,       FLAG_W,                0, 3 }, // 0xC2
	{ INST_RET,    INST_FMT_NONE,      0,                     0, 1 }, // 0xC3
	{ INST_LES,    INST_FMT_RM_REG,    FLAG_D|FLAG_W|FLAG_MO, 0, 2 }, // 0xC4
	{ INST_LDS,    INST_FMT_RM_REG,    FLAG_D|FLAG_W|FLAG_MO, 0, 2 }, // 0xC5
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xC6
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xC7
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0xC8
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0xC9
	{ INST_RETF,   INST_FMT_IMM,       FLAG_W,                0, 3 }, // 0xCA
	{ INST_RETF,   INST_FMT_NONE,      0,                     0, 1 }, // 0xCB
	{ INST_INT3,   INST_FMT_NONE,      0,                     0, 1 }, // 0xCC
	{ INST_INT,    INST_FMT_IMM,       0,                     0, 2 }, // 0xCD
	{ INST_INTO,   INST_FMT_NONE,      0,                     0, 1 }, // 0xCE
	{ INST_IRET,   INST_FMT_NONE,      0,                     0, 1 }, // 0xCF
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xD0
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xD1
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xD2
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xD3
	{ INST_AAM,    INST_FMT_NONE,      0,                     0, 2 }, // 0xD4
	{ INST_AAD,    INST_FMT_NONE,      0,                     0, 2 }, // 0xD5
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0xD6
	{ INST_XLAT,   INST_FMT_NONE,      0,                     0, 1 }, // 0xD7
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xD8
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xD9
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xDA
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xDB
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xDC
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xDD
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xDE
	{ INST_ESC,    INST_FMT_RM_ESC,    FLAG_W,                0, 2 }, // 0xDF
	{ INST_LOOPNZ, INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0xE0
	{ INST_LOOPZ,  INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0xE1
	{ INST_LOOP,   INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0xE2
	{ INST_JCXZ,   INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0xE3
	{ INST_IN,     INST_FMT_ACC_IMM8,  0,                     0, 2 }, // 0xE4
	{ INST_IN,     INST_FMT_ACC_IMM8,  FLAG_W,                0, 2 }, // 0xE5
	{ INST_OUT,    INST_FMT_ACC_IMM8,  0,                     0, 2 }, // 0xE6
	{ INST_OUT,    INST_FMT_ACC_IMM8,  FLAG_W,                0, 2 }, // 0xE7
	{ INST_CALL,   INST_FMT_JMP_NEAR,  0,                     0, 3 }, // 0xE8
	{ INST_JMP,    INST_FMT_JMP_NEAR,  0,                     0, 3 }, // 0xE9
	{ INST_JMP,    INST_FMT_JMP_FAR,   0,                     0, 5 }, // 0xEA
	{ INST_JMP,    INST_FMT_JMP_SHORT, 0,                     0, 2 }, // 0xEB
	{ INST_IN,     INST_FMT_ACC_DX,    0,                     0, 1 }, // 0xEC
	{ INST_IN,     INST_FMT_ACC_DX,    FLAG_W,                0, 1 }, // 0xED
	{ INST_OUT,    INST_FMT_ACC_DX,    0,                     0, 1 }, // 0xEE
	{ INST_OUT,    INST_FMT_ACC_DX,    FLAG_W,                0, 1 }, // 0xEF
	{ INST_LOCK,   INST_FMT_NONE,      0,                     0, 1 }, // 0xF0
	{ INST_UNK,    INST_FMT_NONE,      0,                     0, 1 }, // 0xF1
	{ INST_REPNE,  INST_FMT_NONE,      0,                     0, 1 }, // 0xF2
	{ INST_REP,    INST_FMT_NONE,      0,                     0, 1 }, // 0xF3
	{ INST_HLT,    INST_FMT_NONE,      0,                     0, 1 }, // 0xF4
	{ INST_CMC,    INST_FMT_NONE,      0,                     0, 1 }, // 0xF5
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xF6
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xF7
	{ INST_CLC,    INST_FMT_NONE,      0,                     0, 1 }, // 0xF8
	{ INST_STC,    INST_FMT_NONE,      0,                     0, 1 }, // 0xF9
	{ INST_CLI,    INST_FMT_NONE,      0,                     0, 1 }, // 0xFA
	{ INST_STI,    INST_FMT_NONE,      0,                     0, 1 }, // 0xFB
	{ INST_CLD,    INST_FMT_NONE,      0,                     0, 1 }, // 0xFC
	{ INST_STD,    INST_FMT_NONE,      0,                     0, 1 }, // 0xFD
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xFE
	{ INST_EXTD,   INST_FMT_NONE,      0,                     0, 0 }, // 0xFF
};

struct inst_data inst_table_extd[17][8] =
{
	// [0x00]: 0x80 (0b1000 0000)
	{
		{ INST_ADD,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_OR,   INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_ADC,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_SBB,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_AND,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_SUB,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_XOR,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_CMP,  INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
	},
	// [0x01]: 0x81 (0b1000 0001)
	{
		{ INST_ADD,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_OR,   INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_ADC,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_SBB,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_AND,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_SUB,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_XOR,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_CMP,  INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
	},
	// [0x02]: 0x82 (0b1000 0010)
	{
		{ INST_ADD,  INST_FMT_RM_IMM, FLAG_S,                PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_ADC,  INST_FMT_RM_IMM, FLAG_S,                PFX_WIDE, 3 },
		{ INST_SBB,  INST_FMT_RM_IMM, FLAG_S,                PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_SUB,  INST_FMT_RM_IMM, FLAG_S,                PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_CMP,  INST_FMT_RM_IMM, FLAG_S,                PFX_WIDE, 3 },
	},
	// [0x03]: 0x83 (0b1000 0011)
	{
		{ INST_ADD,  INST_FMT_RM_IMM, FLAG_S|FLAG_W,         PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_ADC,  INST_FMT_RM_IMM, FLAG_S|FLAG_W,         PFX_WIDE, 3 },
		{ INST_SBB,  INST_FMT_RM_IMM, FLAG_S|FLAG_W,         PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_SUB,  INST_FMT_RM_IMM, FLAG_S|FLAG_W,         PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_CMP,  INST_FMT_RM_IMM, FLAG_S|FLAG_W,         PFX_WIDE, 3 },
	},
	// [0x04]: 0x8C (0b1000 1100)
	{
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_ES|FLAG_W,        0,        2 },
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_CS|FLAG_W,        0,        2 },
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_SS|FLAG_W,        0,        2 },
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_DS|FLAG_W,        0,        2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
	},
	// [0x05]: 0x8E (0b1000 1110)
	{
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_ES|FLAG_D|FLAG_W, 0,        2 },
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_CS|FLAG_D|FLAG_W, 0,        2 },
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_SS|FLAG_D|FLAG_W, 0,        2 },
		{ INST_MOV,  INST_FMT_RM_SR,  FLAG_DS|FLAG_D|FLAG_W, 0,        2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
	},
	// [0x06]: 0x8F (0b1000 1111)
	{
		{ INST_POP,  INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
	},
	// [0x07]: 0xC6 (0b1100 0110)
	{
		{ INST_MOV,  INST_FMT_RM_IMM, FLAG_MO,               PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
	},
	// [0x08]: 0xC7 (0b1100 0111)
	{
		{ INST_MOV,  INST_FMT_RM_IMM, FLAG_W|FLAG_MO,        PFX_WIDE, 4 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
	},
	// [0x09]: 0xD0 (0b1101 0000)
	{
		{ INST_ROL,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
		{ INST_ROR,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
		{ INST_RCL,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
		{ INST_RCR,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
		{ INST_SHL,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
		{ INST_SHR,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_SAR,  INST_FMT_RM_V,   0,                     PFX_WIDE, 2 },
	},
	// [0x0A]: 0xD1 (0b1101 0001)
	{
		{ INST_ROL,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
		{ INST_ROR,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
		{ INST_RCL,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
		{ INST_RCR,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
		{ INST_SHL,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
		{ INST_SHR,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_SAR,  INST_FMT_RM_V,   FLAG_W,                PFX_WIDE, 2 },
	},
	// [0x0B]: 0xD2 (0b1101 0010)
	{
		{ INST_ROL,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
		{ INST_ROR,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
		{ INST_RCL,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
		{ INST_RCR,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
		{ INST_SHL,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
		{ INST_SHR,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_SAR,  INST_FMT_RM_V,   FLAG_V,                PFX_WIDE, 2 },
	},
	// [0x0C]: 0xD3 (0b1101 0011)
	{
		{ INST_ROL,  INST_FMT_RM_V,   FLAG_V|FLAG_W,         PFX_WIDE, 2 },
		{ INST_ROR,  INST_FMT_RM_V,   FLAG_V|FLAG_W,         PFX_WIDE, 2 },
		{ INST_RCL,  INST_FMT_RM_V,   FLAG_V|FLAG_W,         PFX_WIDE, 2 },
		{ INST_RCR,  INST_FMT_RM_V,   FLAG_V|FLAG_W,         PFX_WIDE, 2 },
		{ INST_SHL,  INST_FMT_RM_V,   FLAG_V|FLAG_W,         PFX_WIDE, 2 },
		{ INST_SHR,  INST_FMT_RM_V,   FLAG_V|FLAG_W,         PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_SAR,  INST_FMT_RM_V,   FLAG_V| FLAG_W,        PFX_WIDE, 2 },
	},
	// [0x0D]: 0xF6 (0b1111 0110)
	{
		{ INST_TEST, INST_FMT_RM_IMM, 0,                     PFX_WIDE, 3 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_NOT,  INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_NEG,  INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_MUL,  INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_IMUL, INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_DIV,  INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_IDIV, INST_FMT_RM,     0,                     PFX_WIDE, 2 },
	},
	// [0x0E]: 0xF7 (0b1111 0111)
	{
		{ INST_TEST, INST_FMT_RM_IMM, FLAG_W,                PFX_WIDE, 4 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_NOT,  INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
		{ INST_NEG,  INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
		{ INST_MUL,  INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
		{ INST_IMUL, INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
		{ INST_DIV,  INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
		{ INST_IDIV, INST_FMT_RM,     FLAG_W,                PFX_WIDE, 2 },
	},
	// [0x0F]: 0xFE (0b1111 1110)
	{
		{ INST_INC,  INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_DEC,  INST_FMT_RM,     0,                     PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     0,        1 },
	},
	// [0x10]: 0xFF (0b1111 1111)
	{
		{ INST_INC,  INST_FMT_RM,     FLAG_W|FLAG_MO,        PFX_WIDE, 2 },
		{ INST_DEC,  INST_FMT_RM,     FLAG_W|FLAG_MO,        PFX_WIDE, 2 },
		{ INST_CALL, INST_FMT_RM,     FLAG_W,                0,        2 },
		{ INST_CALL, INST_FMT_RM,     FLAG_W|FLAG_MO,        PFX_FAR,  2 },
		{ INST_JMP,  INST_FMT_RM,     FLAG_W,                0,        2 },
		{ INST_JMP,  INST_FMT_RM,     FLAG_W|FLAG_MO,        PFX_FAR,  2 },
		{ INST_PUSH, INST_FMT_RM,     FLAG_W|FLAG_MO,        PFX_WIDE, 2 },
		{ INST_UNK,  INST_FMT_NONE,   0,                     PFX_WIDE, 1 },
	},
};

int get_jmp_offset(struct inst_data data, uint8 * const image, uint size,
                   uint offset)
{
	int    label_addr = 0;
	uint8  tmp8;
	uint16 tmp16;

	switch (data.fmt) {
	case INST_FMT_JMP_SHORT:
		if (offset + 1 > size) return -1;

		tmp8        = image[offset + 1];
		label_addr  = offset + 2;
		label_addr += *((int8_t *)&tmp8);
		break;
	case INST_FMT_JMP_NEAR:
		if (offset + 2 > size) return -2;

		tmp16      = (image[offset + 2] << 8) | image[offset + 1];
		label_addr = offset + 3 + *((int16_t *)&tmp16);
	default:
		return -3;
	}

	return label_addr;
}

int get_inst_data(struct inst_data *data, uint8 * const image, uint size,
                  uint offset)
{
	uint8 extd_op = 0, i = 0;
	struct inst_data tmp;

	tmp = inst_table[image[offset]];
	if (tmp.type == INST_EXTD) {
		switch (image[offset]) {
		case 0x80: i = 0x00; break;
		case 0x81: i = 0x01; break;
		case 0x82: i = 0x02; break;
		case 0x83: i = 0x03; break;
		case 0x8C: i = 0x04; break;
		case 0x8E: i = 0x05; break;
		case 0x8F: i = 0x06; break;
		case 0xC6: i = 0x07; break;
		case 0xC7: i = 0x08; break;
		case 0xD0: i = 0x09; break;
		case 0xD1: i = 0x0A; break;
		case 0xD2: i = 0x0B; break;
		case 0xD3: i = 0x0C; break;
		case 0xF6: i = 0x0D; break;
		case 0xF7: i = 0x0E; break;
		case 0xFE: i = 0x0F; break;
		case 0xFF: i = 0x10; break;
		default:
			assert(image[offset] != image[offset]);
		}

		extd_op = FIELD_EXTD(image[offset + 1]);
		tmp    = inst_table_extd[i][extd_op];
	}

	if (offset + tmp.size > size) return -1;
	tmp.size = calc_disp_size(image, offset);

	if (offset + tmp.size > size) return -2;
	*data = tmp;

	return 0;
}

int inst_scan_image(struct inst_data * const insts, uint count,
                    uint8 * const image, uint size)
{
	int rc = 0;
	int label_addr = 0;
	uint i, offset = 0;
	uint8 prefixes = 0;
	struct inst_data data;
	struct bitmap labels;

	// return instruction count if insts is NULL
	if (!insts) {
		for (count = 0; offset < size; ++count) {
			if (get_inst_data(&data, image, size, offset) < 0)
				return -1;
			if (data.type == INST_UNK) return -2;

			offset += data.size;
		}

		return count;
	}

	if (bitmap_init(&labels, size) < 0) {
		rc = -3;
		goto free_and_exit;
	}

	for (i = 0; i < count && offset < size; ++i) {
		if (get_inst_data(insts + i, image, size, offset) < 0) {
			rc = -1;
			goto free_and_exit;
		}

		if (insts[i].type == INST_UNK) {
			rc = -2;
			goto free_and_exit;
		}

		// handle explicit prefixes
		switch (insts[i].type) {
		case INST_LOCK:
			prefixes |= PFX_LOCK;
			break;
		case INST_SGMNT:
			prefixes |= insts[i].flags;
			prefixes |= PFX_SGMNT;
			break;
		case INST_REP:
			prefixes |= PFX_REP;
			break;
		case INST_REPNE:
			prefixes |= PFX_REPNE;
			break;
		// if it isn't a prefix instruction, assign accumulated prefixes
		default:
			insts[i].prefixes = prefixes;
			prefixes = 0;
		}

		label_addr = get_jmp_offset(insts[i], image, size, offset);
		if (label_addr >= 0) {
			bitmap_set_bit(&labels, label_addr);
		}

		offset += insts[i].size;
	}

	// setting FLAG_LB flag for label generation
	for (i = 0, offset = 0; i < count && offset < size; ++i) {
		if (bitmap_get_bit(&labels, offset) > 0) {
			insts[i].flags |= FLAG_LB;
		}

		offset += insts[i].size;
	}

free_and_exit:
	bitmap_free(&labels);

	return rc;
}

uint calc_disp_size(uint8 * const image, uint offset)
{
	uint8 mod, r_m;
	uint8 *inst = image + offset;

	mod = FIELD_MOD(inst[1]);
	r_m = FIELD_RM(inst[1]);

	// direct address and 16-bit displacement
	if ((mod == 0b00 && r_m == 0b110) || mod == 0b10)
		return 2;

	// 8-bit displacement
	if (mod == 0b01)
		return 1;

	return 0;
}

