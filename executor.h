#if !defined EXECUTOR_H
#define EXECUTOR_H

#include "common.h"
#include "inst.h"

union reg
{
	struct
	{
		uint8 low;
		uint8 high;
	};
	uint16 word;
};

struct cpu_state
{
	union
	{
		struct
		{
			uint16 ax;
			uint16 cx;
			uint16 dx;
			uint16 bx;
			uint16 sp;
			uint16 bp;
			uint16 si;
			uint16 di;
		};

		uint8  regs8[8][2];
		uint16 regs16[8];
	};

	union
	{
		struct
		{
			uint16 es;
			uint16 cs;
			uint16 ss;
			uint16 ds;
		};

		uint16 segregs[4];
	};

	uint16 ip;
};

extern int executor_init_state(struct cpu_state *state);
extern int executor_exec(struct cpu_state *state, struct inst *inst);

#endif /* EXECUTOR_H */
