#ifndef __simplerecorder__
#define __simplerecorder__
#include "picture_t.h"

void *input_init(char *filename, struct picture_t *pic);
int input_getframe(void *state, struct picture_t *pic);

int output_init(struct picture_t *info, const char *str);
int output_write_headers(struct encoded_pic_t *headers);
int output_write_frame(struct encoded_pic_t *encoded);
void output_close();

int encoder_init(struct picture_t *info);
int encoder_encode_headers(struct encoded_pic_t *headers_out);
int encoder_encode_frame(struct picture_t *raw_pic, struct encoded_pic_t *output);
void encoder_release(struct encoded_pic_t *output);
void encoder_close();
void ResetTime(struct picture_t *raw_pic,struct encoded_pic_t *output);
#endif // __simplerecorder__
