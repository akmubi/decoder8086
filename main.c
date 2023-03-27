#include <stdio.h>
#include <stdlib.h>

#include "inst.h"
#include "decoder.h"

int main(int argc, char *argv[])
{
	int i, rc = 0;
	uint nread, size = 0;
	FILE *file   = NULL;
	uint8 *image = NULL;

	uint offset = 0;
	int inst_count = 0;
	struct inst_data *insts = NULL;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <assembled-file>\n", argv[0]);
		return 1;
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

	fprintf(stdout, "; %s\nbits 16\n\n", argv[1]);

	for (i = 0, offset = 0; i < inst_count && offset < size; ++i) {
		rc = decode_inst(stdout, insts[i], image, size, offset);
		if (rc < 0) {
			fprintf(stderr, "failed to decode instruction "
			                "(exit code %d)\n", rc);
			return -7;
		}

		switch (insts[i].type) {
		case INST_LOCK:
		case INST_REP:
		case INST_REPNE:
			fputc(' ', stdout);
			break;
		case INST_SGMNT:
			break;
		default:
			fputc('\n', stdout);
		}

		offset += insts[i].size;
	}

	return 0;
}
