#if !defined EXECUTOR_H
#define EXECUTOR_H

#include "common.h"
#include "inst.h"

union general_reg
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
			union general_reg ax;
			union general_reg cx;
			union general_reg dx;
			union general_reg bx;
			uint16 sp;
			uint16 bp;
			uint16 si;
			uint16 di;
		};
		uint16 regs[8];
	};

	uint16 ip;
};

extern int executor_init_state(struct cpu_state *state);
extern int executor_exec(struct cpu_state *state, struct inst_data *data,
                         uint8 * const image, uint size, uint offset);

#endif /* EXECUTOR_H */
