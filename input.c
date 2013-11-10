#include <stdio.h>
#include <stdlib.h>
#include "picture_t.h"
#include "simplerecorder.h"

void *input_init(struct picture_t *pic)
{
	// read input data from stdin; pic contains width and height info
	// this version does nothing, but other input classes might

	return (void *) stdin;
}

int input_getframe(void *state, struct picture_t *pic)
{
	FILE *fp = (FILE *) state;
	if (!fp)
		return 0;

	return fread(pic->buffer, 1, pic->width * pic->height / 4 * 6, fp);
}
