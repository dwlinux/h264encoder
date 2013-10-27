#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "picture_t.h"
#include "simplerecorder.h"

#define mkv_filename "output.mkv"
static struct picture_t pic;

int main()
{
	int i;
	struct encoded_pic_t encoded_pic,header_pic;

	memset(&pic, 0, sizeof(pic));

	if (!input_init(&pic))
		goto error_input;

	fprintf(stderr, "width: %d, height: %d\n", pic.width, pic.height);

	pic.buffer = malloc(pic.width * pic.height / 4 * 6);
	if (!pic.buffer)
		goto error_input;

	input_getframe(0, &pic);

	if(!encoder_init(&pic)){
		fprintf(stderr,"failed to initialize encoder\n");
		goto error_encoder;
	}
	if(!output_init(&pic,mkv_filename))
		goto error_output;
	if(!encoder_encode_headers(&encoded_pic))
		goto error_output;
	memcpy(&header_pic,&encoded_pic,sizeof(encoded_pic));
	header_pic.buffer=malloc(encoded_pic.length);
	memcpy(header_pic.buffer,encoded_pic.buffer,encoded_pic.length);
	if(!output_write_headers(&header_pic))
		goto error_output;

	// not sure about this -- but g_outputDataInfo.uSize0 appears to
	// contain a whole frame at this point, so write it out and start
	// at frame 1 in the loop
	if(!output_write_frame(&encoded_pic))
		goto error_encoder;

	encoder_release(&encoded_pic);

	for(i=1 ;; i++){
		if (!input_getframe(i, &pic))
			goto no_error;
		pic.timestamp.tv_sec = i / 25;
		pic.timestamp.tv_usec = (i * 40000) % 1000000;	// 25 FPS
		if(!encoder_encode_frame(&pic, &encoded_pic))
			goto error_encoder;
		if(!output_write_frame(&encoded_pic))
			goto error_encoder;
		encoder_release(&encoded_pic);
	}

error_input:
	fprintf(stderr, "error input\n");
	goto no_error;
error_output:
	fprintf(stderr, "error output\n");
	goto no_error;
error_encoder:
	encoder_release(&encoded_pic);
	fprintf(stderr, "error encoder\n");
	goto no_error;
no_error:
	fprintf(stderr, "%d frames recorded\n", i);
	if (pic.buffer)
		free(pic.buffer);
	return 0;
}
