#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "picture_t.h"
#include "simplerecorder.h"

#define mkv_filename "output.mkv"
static struct picture_t pic;

int main()
{
	int i;
	struct encoded_pic_t encoded_pic,header_pic;
	char filename[128];
	FILE *fp;

	memset(&pic, 0, sizeof(pic));

	// read input data from files named frame.0, frame.1, frame.2, ...
	sprintf(filename, "frame.%d", 0);
	fp = fopen(filename, "r");
	if (!fp)
		goto error_input;

	// input file format:
	//    width:4
	//    height:4
	//    data:width*height/4*6
	//        YUV420p image format
	// note: width should probably be a multiple of 4 for now
	fread(&pic.width, sizeof(int), 1, fp);
	fread(&pic.height, sizeof(int), 1, fp);
	if (!pic.width || !pic.height) {
		fclose(fp);
		goto error_input;
	}
	pic.buffer = malloc(pic.width * pic.height / 4 * 6);
	if (!pic.buffer) {
		fclose(fp);
		goto error_input;
	}
	fclose(fp);

	if(!encoder_init(&pic)){
		fprintf(stderr,"failed to initialize encoder\n");
		goto error_encoder;
	}
	printf("file:%s\n",mkv_filename);
	if(!output_init(&pic,mkv_filename))
		goto error_output;
	if(!encoder_encode_headers(&encoded_pic))
		goto error_output;
	memcpy(&header_pic,&encoded_pic,sizeof(encoded_pic));
	header_pic.buffer=malloc(encoded_pic.length);
	memcpy(header_pic.buffer,encoded_pic.buffer,encoded_pic.length);
	if(!output_write_headers(&header_pic))
		goto error_output;
	encoder_release(&encoded_pic);
	for(i=0 ;; i++){
		int width = 0, height = 0;
		sprintf(filename, "frame.%d", i);
		fp = fopen(filename, "r");
		if (!fp)
			goto no_error;
		fread(&width, sizeof(int), 1, fp);
		fread(&height, sizeof(int), 1, fp);
		if (width != pic.width || height != pic.height) {
			fclose(fp);
			goto error_input;
		}

		fread(pic.buffer, 1, pic.width * pic.height / 4 * 6, fp);
		fclose(fp);
		
		pic.timestamp.tv_sec = i / 25;
		pic.timestamp.tv_usec = i * 40000;	// 25 FPS
		if(!encoder_encode_frame(&pic, &encoded_pic))
			goto error_encoder;
		if(!output_write_frame(&encoded_pic))
			goto error_encoder;
		encoder_release(&encoded_pic);
	}

error_input:
	printf("error input\n");
	goto no_error;
error_output:
	printf("error output\n");
	goto no_error;
error_encoder:
	encoder_release(&encoded_pic);
	printf("error encoder\n");
	goto no_error;
no_error:
	printf("\n%d frames recorded\n", i);
	if (pic.buffer)
		free(pic.buffer);
	return 0;
}
