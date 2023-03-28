#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"
#include "decoder.h"
#include "executor.h"

#define FLAG_EXEC "-i"

void usage(char *argv[])
{
	fprintf(stderr, "Usage: %s <assembled-file> [-i]\n"
	        "\t-i\texecute instuctions\n", argv[0]);
}

int main(int argc, char *argv[])
{
	int i, rc = 0;
	uint nread, size = 0;

	FILE *file   = NULL;
	uint8 *image = NULL;

	uint offset = 0;
	int inst_count = 0;
	struct inst *insts = NULL;
	struct cpu_state state;

	if (argc < 2) {
		usage(argv);
		return 1;
	}

	if (argc == 3 && strcmp(argv[2], FLAG_EXEC)) {
		usage(argv);
		return 2;
	}

	file = fopen(argv[1], "rb");
	if (!file) {
		perror("failed to open file");
		return -1;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	image = malloc(size);
	if (!image) {
		perror("failed to allocate buffer for image");
		return -2;
	}

	nread = fread(image, 1, size, file);
	if (nread != size) {
		perror("failed to read from file");
		return -3;
	}

	fclose(file);

	inst_count = inst_scan_image(NULL, 0, image, size);
	if (inst_count < 0) {
		fprintf(stderr, "failed to scan image for instructions "
		        "(exit code %d)\n", inst_count);
		return -4;
	}

	insts = malloc(inst_count * sizeof(*insts));
	if (!insts) {
		perror("failed to allocate array for instructions");
		return -5;
	}

	rc = inst_scan_image(insts, inst_count, image, size);
	if (rc < 0) {
		fprintf(stderr, "failed to scan image for instructions "
		                "(exit code %d)\n", rc);
		return -6;
	}

	executor_init_state(&state);

	fprintf(stdout, "; %s\nbits 16\n\n", argv[1]);

	for (i = 0, offset = 0;
	     i < inst_count && offset < size;
	     ++i, offset += insts[i].base.size) {
		rc = decode_inst(stdout, insts + i);
		if (rc < 0) {
			fprintf(stderr, "failed to decode instruction "
			                "(exit code %d)\n", rc);
			return -7;
		}

		switch (insts[i].base.type) {
		case INST_SGMNT:
			continue;
		case INST_LOCK:
		case INST_REP:
		case INST_REPNE:
			fputc(' ', stdout);
			continue;
		default:
			break;
		}

		fputc('\n', stdout);

		if (argc == 3) {
			executor_exec(&state, insts + i);

			printf("; ax: %04X cx: %04X dx: %04X bx: %04X\n",
			       state.ax, state.cx, state.dx, state.bx);
			printf("; sp: %04X bp: %04X si: %04X di: %04X\n",
			       state.sp, state.bp, state.si, state.di);
			printf("; es: %04X cs: %04X ss: %04X ds: %04X\n",
			       state.es, state.cs, state.ss, state.ds);
		}
	}

	return 0;
}
