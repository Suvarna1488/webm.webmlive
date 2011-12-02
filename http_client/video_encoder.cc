// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "http_client/video_encoder.h"

#include <new>

#include "glog/logging.h"

namespace webmlive {

VideoFrame::VideoFrame()
    : width_(0), height_(0), timestamp_(0), buffer_length_(0) {
}

VideoFrame::~VideoFrame() {
}

int32 VideoFrame::InitI420(int32 width, int32 height, int64 timestamp,
                           uint8* ptr_data, int32 data_length) {
  if (!ptr_data) {
    return kInvalidArg;
  }
  width_ = width;
  height_ = height;
  timestamp_ = timestamp;
  if (data_length > buffer_length_) {
    buffer_length_ = data_length;
    buffer_.reset(new (std::nothrow) uint8[buffer_length_]);
    if (!buffer_) {
      return kNoMemory;
    }
  }
  memcpy(&buffer_[0], ptr_data, data_length);
  return kSuccess;
}

void VideoFrame::Swap(VideoFrame* ptr_frame) {
  CHECK_NOTNULL(buffer_.get());
  CHECK_NOTNULL(ptr_frame->buffer_.get());
  CHECK_EQ(buffer_length_, ptr_frame->buffer_length_);
  int32 temp = width_;
  width_ = ptr_frame->width_;
  ptr_frame->width_ = temp;

  temp = height_;
  height_ = ptr_frame->height_;
  ptr_frame->height_ = temp;

  int64 temp_timestamp = timestamp_;
  timestamp_ = ptr_frame->timestamp_;
  ptr_frame->timestamp_ = temp_timestamp;

  buffer_.swap(ptr_frame->buffer_);

  temp = buffer_length_;
  buffer_length_ = ptr_frame->buffer_length_;
  ptr_frame->buffer_length_ = temp;
}

#if 0
    FILE                *infile, *outfile;
    vpx_codec_ctx_t      codec;
    vpx_codec_enc_cfg_t  cfg;
    int                  frame_cnt = 0;
    vpx_image_t          raw;
    vpx_codec_err_t      res;
    long                 width;
    long                 height;
    int                  frame_avail;
    int                  got_data;
    int                  flags = 0;

    /* Open files */
    if(argc!=5)
        die("Usage: %s <width> <height> <infile> <outfile>\n", argv[0]);
    width = strtol(argv[1], NULL, 0);
    height = strtol(argv[2], NULL, 0);
    if(width < 16 || width%2 || height <16 || height%2)
        die("Invalid resolution: %ldx%ld", width, height);
    if(!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1))
        die("Faile to allocate image", width, height);
    if(!(outfile = fopen(argv[4], "wb")))
        die("Failed to open %s for writing", argv[4]);

    printf("Using %s\n",vpx_codec_iface_name(interface));

    /* Populate encoder configuration */                                      //
    res = vpx_codec_enc_config_default(interface, &cfg, 0);                   //
    if(res) {                                                                 //
        printf("Failed to get config: %s\n", vpx_codec_err_to_string(res));   //
        return EXIT_FAILURE;                                                  //
    }                                                                         //

    /* Update the default configuration with our settings */                  //
    cfg.rc_target_bitrate = width * height * cfg.rc_target_bitrate            //
                            / cfg.g_w / cfg.g_h;                              //
    cfg.g_w = width;                                                          //
    cfg.g_h = height;                                                         //

    write_ivf_file_header(outfile, &cfg, 0);


        /* Open input file for this encoding pass */
        if(!(infile = fopen(argv[3], "rb")))
            die("Failed to open %s for reading", argv[3]);

        /* Initialize codec */                                                //
        if(vpx_codec_enc_init(&codec, interface, &cfg, 0))                    //
            die_codec(&codec, "Failed to initialize encoder");                //

        frame_avail = 1;
        got_data = 0;
        while(frame_avail || got_data) {
            vpx_codec_iter_t iter = NULL;
            const vpx_codec_cx_pkt_t *pkt;

            frame_avail = read_frame(infile, &raw);                           //
            if(vpx_codec_encode(&codec, frame_avail? &raw : NULL, frame_cnt,  //
                                1, flags, VPX_DL_REALTIME))                   //
                die_codec(&codec, "Failed to encode frame");                  //
            got_data = 0;
            while( (pkt = vpx_codec_get_cx_data(&codec, &iter)) ) {
                got_data = 1;
                switch(pkt->kind) {
                case VPX_CODEC_CX_FRAME_PKT:                                  //
                    write_ivf_frame_header(outfile, pkt);                     //
                    if(fwrite(pkt->data.frame.buf, 1, pkt->data.frame.sz,     //
                              outfile));                                      //
                    break;                                                    //
                default:
                    break;
                }
                printf(pkt->kind == VPX_CODEC_CX_FRAME_PKT
                       && (pkt->data.frame.flags & VPX_FRAME_IS_KEY)? "K":".");
                fflush(stdout);
            }
            frame_cnt++;
        }
        printf("\n");
        fclose(infile);

    printf("Processed %d frames.\n",frame_cnt-1);
    if(vpx_codec_destroy(&codec))                                             //
        die_codec(&codec, "Failed to destroy codec");                         //

    /* Try to rewrite the file header with the actual frame count */
    if(!fseek(outfile, 0, SEEK_SET))
        write_ivf_file_header(outfile, &cfg, frame_cnt-1);
    fclose(outfile);
    return EXIT_SUCCESS;
#endif

}  // namespace webmlive
