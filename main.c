#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include "picture_t.h"
#include "simplerecorder.h"
#define MAX_SIZE 1024*1024*10

static char mkv_filename[100];
static char dirname[20];
static char tempname[20];
static struct picture_t pic;

static void get_filename()
{
	time_t t = time(0);
	strcpy(mkv_filename,"./Record/");
	strftime(dirname, 20, "%Y%m%d",localtime(&t));
	strftime(tempname, 20, "%H%M%S",localtime(&t));
	strcat(mkv_filename,dirname);
	if(NULL==opendir(mkv_filename))
		mkdir(mkv_filename,0775);
	strcat(mkv_filename,"/");
	strcat(mkv_filename,tempname);
	strcat(mkv_filename,".mkv");
}

int main()
{
	int i,FileSize;
	struct timeval tpstart;
	struct encoded_pic_t encoded_pic,header_pic;
	errno = 0;
	if(NULL==opendir("./Record"))
		mkdir("./Record",0775);
	if(!encoder_init(&pic)){
		fprintf(stderr,"failed to initialize encoder\n");
		goto error_encoder;
	}
	get_filename();
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
	FileSize =0;
	for(i=0 ;; i++){
		char filename[128];
		FILE *fp;

		memset(&pic, 0, sizeof(pic));

		// read input data from files named frame.0, frame.1, frame.2, ...
		sprintf(filename, "frame.%d", i);
		fp = fopen(filename, "r");
		if (!fp)
			break;

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
			break;
		}
		pic.buffer = malloc(pic.width * pic.height / 4 * 6);
		if (!pic.buffer) {
			fclose(fp);
			break;
		}
		fread(pic.buffer, 1, pic.width * pic.height / 4 * 6, fp);
		fclose(fp);
		
		gettimeofday(&tpstart,NULL);
		pic.timestamp = tpstart;
		FileSize+=encoded_pic.length;
		if(!encoder_encode_frame(&pic, &encoded_pic)) {
			free(pic.buffer);
			break;
		}
		free(pic.buffer);
		if ((FileSize>MAX_SIZE) && (encoded_pic.frame_type ==FRAME_TYPE_I)) {
			output_close();	
			get_filename();
			printf("file:%s\n",mkv_filename);
			if(!output_init(&pic,mkv_filename))
				goto error_output;
			if(!output_write_headers(&header_pic))
				goto error_output;
			FileSize=0;
			ResetTime(&pic,&encoded_pic);
			if(!output_write_frame(&encoded_pic))
				break;
			encoder_release(&encoded_pic);
		} else {
			if(!output_write_frame(&encoded_pic))
				break;
			encoder_release(&encoded_pic);
		}
	}
	printf("\n%d frames recorded\n", i);

error_output:
	printf("error output\n");
error_encoder:
	return 0;
}
