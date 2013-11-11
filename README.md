h264encoder
==============

* encoder.c:     H264 encoder
* output.c:      generate mkv output
* input.c:       reads NV12 input from stdin or optional frame file
* main.c:        overall encoder control

Sample usage:

    avconv -i <input_video_file> -vf pad="trunc((iw+31)/32)*32" \
        -pix_fmt nv12 -f rawvideo pipe: |
        simplerecorder <width> <height>
---
