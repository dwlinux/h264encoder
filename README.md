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
        simpleencoder.static -w <width> -h <height> [-o <output_file>]

---
