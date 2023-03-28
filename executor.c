#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "executor.h"
#include "inst.h"

static void execute_mov(struct cpu_state *state, struct inst *inst);

int executor_init_state(struct cpu_state *state)
{
	if (!state) {
		fprintf(stderr, "invalid arguments (state: %p)\n", state);
		return -1;
	}

	memset(state, 0, sizeof(*state));

	return 0;
}

int executor_exec(struct cpu_state *state, struct inst *inst)
{
	if (!state || !inst) {
		fprintf(stderr, "invalid arguments (state: %p, inst: %p)\n",
		        state, inst);
		return -1;
	}

	switch (inst->base.type) {
	case INST_MOV:
		execute_mov(state, inst); break;
	default:
		assert(0 && "");
	}

	return 0;
}

void execute_mov(struct cpu_state *state, struct inst *inst)
{
	uint8   mod, rm, reg, sr;
	uint16 *reg_reg16 = NULL, *rm_reg16 = NULL;
	uint8  *reg_reg8  = NULL, *rm_reg8  = NULL;
	uint16 *sr_reg    = NULL;
	void   *dest      = NULL, *src = NULL;

	mod = FIELD_MOD(inst->fields);
	rm  = FIELD_RM(inst->fields);
	reg = FIELD_REG(inst->fields);
	sr  = FIELD_SR(inst->fields);

	rm_reg8   = &state->regs8[rm  & 0b11][rm  >> 2];
	reg_reg8  = &state->regs8[reg & 0b11][reg >> 2];

	rm_reg16  = &state->regs16[rm];
	reg_reg16 = &state->regs16[reg];

	sr_reg    = &state->segregs[sr];

	switch (inst->base.fmt) {
	case INST_FMT_RM_REG:
		if (mod != MODE_REG)
			assert(0 && "not implemented");

		if (inst->base.flags & F_W) {
			dest = rm_reg16; src = reg_reg16;
			if (inst->base.flags & F_D) {
				dest = reg_reg16; src = rm_reg16;
			}

			*((uint16 *)dest) = *((uint16 *)src);
		} else {
			dest = rm_reg8; src = reg_reg8;
			if (inst->base.flags & F_D) {
				dest = reg_reg8; src = rm_reg8;
			}

			*((uint8 *)dest) = *((uint8 *)src);
		}

		break;
		
	case INST_FMT_ACC_MEM:
	case INST_FMT_REG_IMM:
		if (inst->base.flags & F_W) {
			*reg_reg16 = inst->data;
		} else {
			*reg_reg8  = inst->data & 0xFF;
		}

		break;
	case INST_FMT_RM_SR:
		if (mod != MODE_REG)
			assert(0 && "not implemented");

		if (inst->base.flags & F_D) {
			*sr_reg = *rm_reg16;
		} else {
			*rm_reg16 = *sr_reg;
		}

		break;
	case INST_FMT_RM_IMM:
		if (mod != MODE_REG)
			assert(0 && "not implemented");

		if (inst->base.flags & F_W) {
			*rm_reg16 = inst->data;
		} else {
			*rm_reg8  = inst->data & 0xFF;
		}
	default:
		break;
	}
}
