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
static int option_nframes	= 0;

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s [options] [rawfile]\n", name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h,  --help\n");
	fprintf(stderr, "\t-o,  --output=<filename>\n");
	fprintf(stderr, "\t-s,  --size=<width>x<height>    (required)\n");
	fprintf(stderr, "\t-n,  --nframes=<number>         encode only first <number> of frames");
	fprintf(stderr, "\n");
	exit(1);
}

static struct option long_options[] = {
	{"help",	no_argument,		0,	'h'},
	{"output",	required_argument,	0,	'o'},
	{"size",	required_argument, 	0,	's'},
	{"nframes",	required_argument,	0, 	'n'},
	{0, 0, 0, 0}
};

static void parse_options(int argc, char **argv)
{
	int c;
	int index = 0;
	while(1) {
		c = getopt_long(argc, argv, "ho:s:n:", long_options, &index);
		if(c < 0)
			break;

		if(optarg && optarg[0] == '-')
			c = -1;

		switch(c) {
			case 'o':
				option_output_filename = optarg;
				break;
			case 's':
				if(2 != sscanf(optarg, "%dx%d", &option_width, &option_height))
					usage(argv[0]);
				break;
			case 'n':
				option_nframes = atoi(optarg);
				break;
			case 'h':
			default:
				usage(argv[0]);
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
	}

	pic.width  = option_width;
	pic.height = option_height;

	if (0 > (input_state = input_init(option_input_filename, &pic)))
		goto error_input;

	fprintf(stderr, "rawvideo: %s\n", option_input_filename);
	fprintf(stderr, "width: %d, height: %d\n", pic.width, pic.height);
	fprintf(stderr, "output: %s\n", option_output_filename);
	fprintf(stderr, "nframes: %d\n", option_nframes);

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

	for(i=0 ;; i++){
		if (!input_getframe(input_state, &pic))
			goto no_error;
		pic.timestamp.tv_sec = i / 25;
		pic.timestamp.tv_usec = (i * 40000) % 1000000;	// 25 FPS
		if(!encoder_encode_frame(&pic, &encoded_pic, &header_pic))
			goto error_encoder;

		if(option_output_filename) {
			if(header_pic.length > 0) {
				if(!output_write_headers(&header_pic))
					goto error_output;
			}
			if(!output_write_frame(&encoded_pic))
				goto error_encoder;
		}

		encoder_release(&encoded_pic);

		if(option_nframes > 0 && (i + 1) >= option_nframes)
			break;
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
	fprintf(stderr, "%d frames recorded\n", i + 1);
	return 0;
}
