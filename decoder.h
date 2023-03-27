#if !defined DECODER_H
#define DECODER_H

#include <stdio.h>

#include "common.h"
#include "inst.h"

// Decodes an instruction and writes string representation into out file.
// Returns 0 on success and non-zero value if an error occurred.
extern int decode_inst(FILE *out, struct inst *inst);

#endif /* DECODER_H */
