h264encoder
==============

* encoder.c:     H264 encoder
* output.c:      generate mkv output
* input.c:       reads NV12 input from stdin or optional frame file
* main.c:        overall encoder control

Both static and dynamic versions are built.  The static version can
be run on both armel and armhf distributions.

Sample usage:

    avconv -i <input_video_file> -vf pad="trunc((iw+31)/32)*32" \
        -pix_fmt nv12 -f rawvideo pipe: |
        simpleencoder.static -s <width>x<height> [-o <output_file>]

With previous converted raw video files:

    avconv -i <input_video_file> -vf pad="trunc((iw+31)/32)*32" \
        -pix_fmt nv12 -f rawvideo <output_raw_video_file>

If the end of the raw file name is as:

    filename_<width>x<height>.nv12

Can be used as follows:

    simpleencoder.static <output_raw_video_file> [-o <output_file>]

---
