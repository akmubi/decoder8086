#if !defined DECODER_H
#define DECODER_H

#include <stdio.h>

#include "common.h"
#include "inst.h"

// Decodes next instruction starting from image[offset], writes string
// representation into out file and sets decoded instruction size.
// Returns 0 on success and non-zero value if an error occurred
extern int decode_inst(FILE *out, struct inst_data data, uint8 * const image,
                       uint size, uint offset);

#endif /* DECODER_H */
