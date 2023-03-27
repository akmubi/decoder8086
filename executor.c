#include <stdio.h>
#include <string.h>

#include "executor.h"

static void execute_mov(struct cpu_state *state, struct inst_data *data,
                        uint8 * const image, uint size, uint offset);

int executor_init_state(struct cpu_state *state)
{
	if (!state) {
		fprintf(stderr, "invalid arguments (state: %p)\n", state);
		return -1;
	}

	memset(state, 0, sizeof(*state));

	return 0;
}

int executor_exec(struct cpu_state *state, struct inst_data *data,
                  uint8 * const image, uint size, uint offset)
{
	if (!state || !data || !image) {
		fprintf(stderr, "invalid arguments (state: %p, data: %p, "
		        "image: %p)\n", state, data, image);
		return -1;
	}

	if (offset + data->size > size) {
		fprintf(stderr, "");
		return -2;
	}

	return 0;
}

static void execute_mov(struct cpu_state *state, struct inst_data *data,
                        uint8 * const image, uint size, uint offset);
