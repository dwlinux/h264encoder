#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "picture_t.h"
#include "simplerecorder.h"

static struct picture_t pic;
static char *option_input_filename  = NULL;
static char *option_output_filename = NULL;
static int option_width		= 0;
static int option_height	= 0;

static void usage(char *name)
{
	fprintf(stderr, "usage: %s [options] [rawfile]\n", name);
	fprintf(stderr, "\t-w width,    -width=width     (required)\n");
	fprintf(stderr, "\t-h height,   -height=height   (required)\n");
	fprintf(stderr, "\t-o filename, -output=filename\n");
	fprintf(stderr, "\n");
}

static struct option long_options[] = {
	{"output",	required_argument,	0,	'o'},
	{"width",	required_argument, 	0,	'w'},
	{"height",	required_argument,	0, 	'h'},
	{0, 0, 0, 0}
};

static void parse_options(int argc, char **argv)
{
	int c;
	int index = 0;
	while(1) {
		c = getopt_long(argc, argv, "o:w:h:", long_options, &index);
		if(c < 0)
			break;

		if(optarg && optarg[0] == '-')
			c = -1;

		switch(c) {
			case 'o':
				option_output_filename = optarg;
				break;
			case 'w':
				option_width = atoi(optarg);
				break;
			case 'h':
				option_height = atoi(optarg);
				break;
			default:
				usage(argv[0]);
				exit(1);
				break;
		}
	}
	if(optind < argc)
		option_input_filename = argv[optind];
}

int main(int argc, char **argv)
{
	int i;
	struct encoded_pic_t encoded_pic,header_pic;
	void *input_state;

	parse_options(argc, argv);


	memset(&pic, 0, sizeof(pic));

	if (!option_width || !option_height) {
		usage(argv[0]);
		exit(1);
	}

	pic.width  = option_width;
	pic.height = option_height;

	if (0 > (input_state = input_init(option_input_filename, &pic)))
		goto error_input;

	fprintf(stderr, "rawvideo: %s\n", option_input_filename);
	fprintf(stderr, "width: %d, height: %d\n", pic.width, pic.height);
	fprintf(stderr, "output: %s\n", option_output_filename);

	if(!encoder_init(&pic)){
		fprintf(stderr,"failed to initialize encoder\n");
		goto error_encoder;
	}

	if (!pic.buffer)
		goto error_input;
	input_getframe(input_state, &pic);

	if(option_output_filename)
		if(!output_init(&pic, option_output_filename))
			goto error_output;
	
	if(!encoder_encode_headers(&encoded_pic))
		goto error_output;
	
	if(option_output_filename) {
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
	}
	encoder_release(&encoded_pic);

	for(i=1 ;; i++){
		if (!input_getframe(input_state, &pic))
			goto no_error;
		pic.timestamp.tv_sec = i / 25;
		pic.timestamp.tv_usec = (i * 40000) % 1000000;	// 25 FPS
		if(!encoder_encode_frame(&pic, &encoded_pic))
			goto error_encoder;

		if(option_output_filename) {
			if(!output_write_frame(&encoded_pic))
				goto error_encoder;
		}

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
	if(option_output_filename)
		output_close();
	fprintf(stderr, "%d frames recorded\n", i);
	return 0;
}
