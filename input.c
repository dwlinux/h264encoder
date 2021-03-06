#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "picture_t.h"
#include "simplerecorder.h"

void *input_init(char *filename, struct picture_t *pic)
{
	// read input data from stdin; pic contains width and height info
	// this version does nothing, but other input classes might
	int fd = 0;
	int width  = 0;
	int height = 0;
	char format[5]  = "";
	char *extension = NULL;
	if (filename) {
		if (pic->width == 0 && pic->height == 0) {
			if ((extension = strrchr(filename, '_'))) {
				if (3 == sscanf(extension, "_%dx%d.%4s", &width, &height, format)) {
					fprintf(stderr, "input: %dx%d %s\n", width, height, format);
					pic->width  = width;
					pic->height = height;
				}
			}
		}

		fd = open(filename, O_RDONLY);
	}

	return (void *) fd;
}

int input_getframe(void *state, struct picture_t *pic)
{
	int fd = (int) state;
	if (fd < 0)
		return 0;

	int size = pic->width * pic->height / 4 * 6;
	int reed = 0;
	int retv;

	while(reed < size) {
		retv = read(fd, pic->buffer + reed, size - reed);
		if (retv <= 0)
			return retv;
		reed += retv;
	}

	return reed;
}
