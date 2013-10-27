#include <stdio.h>
#include <stdlib.h>
#include "picture_t.h"
#include "simplerecorder.h"

int input_init(struct picture_t *pic)
{
	// read input data from files named frame.0, frame.1, frame.2, ...
	FILE *fp = fopen("frame.0", "r");
	if (!fp)
		return 0;

	// input file format:
	//    width:4
	//    height:4
	//    data:width*height/4*6
	//        YUV420p image format
	// note: width should probably be a multiple of 4 for now
	fread(&pic->width, sizeof(int), 1, fp);
	fread(&pic->height, sizeof(int), 1, fp);
	fclose(fp);
	if (!pic->width || !pic->height)
		return 0;

	return 1;
}

int input_getframe(int frame, struct picture_t *pic)
{
	char filename[128];
	int width = 0, height = 0;

	sprintf(filename, "frame.%d", frame);
	FILE *fp = fopen(filename, "r");
	if (!fp)
		return 0;

	fread(&width, sizeof(int), 1, fp);
	fread(&height, sizeof(int), 1, fp);
	if (width != pic->width || height != pic->height) {
		fclose(fp);
		return 0;
	}

	fread(pic->buffer, 1, pic->width * pic->height / 4 * 6, fp);
	fclose(fp);
	return 1;
}
