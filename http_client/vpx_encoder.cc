// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#if defined _MSC_VER
// Disable warning C4505(unreferenced local function has been removed) in MSVC.
// At the time this comment was written the warning is emitted 27 times for
// vp8.h and vp8cx.h (included by vpx_encoder.h).
#pragma warning(disable:4505)
#endif
#include "http_client/vpx_encoder.h"

#include "glog/logging.h"
#include "http_client/webm_encoder.h"

namespace webmlive {

VpxEncoder::VpxEncoder() {
  memset(&vp8_context_, 0, sizeof(vp8_context_));
}

VpxEncoder::~VpxEncoder() {
}

int VpxEncoder::Init(const WebmEncoderConfig* ptr_user_config) {
  if (!ptr_user_config) {
    LOG(ERROR) << "VpxEncoder cannot Init with NULL VpxConfig!";
    return kInvalidArg;
  }
  vpx_codec_enc_cfg_t vp8_config = {0};
  vpx_codec_err_t status = vpx_codec_enc_config_default(vpx_codec_vp8_cx(),
                                                        &vp8_config, 0);
  if (status) {
    LOG(ERROR) << "vpx_codec_enc_config_default failed: "
               << vpx_codec_err_to_string(status);
    return VideoEncoder::kCodecError;
  }
  vp8_config.g_h = ptr_user_config->video_config.height;
  vp8_config.g_w = ptr_user_config->video_config.width;
  const VpxConfig& user_vpx_config = ptr_user_config->vpx_config;
  vp8_config.g_threads = user_vpx_config.thread_count;
  vp8_config.rc_target_bitrate = user_vpx_config.bitrate;
  vp8_config.rc_min_quantizer = user_vpx_config.min_quantizer;
  vp8_config.rc_max_quantizer = user_vpx_config.max_quantizer;




  return kSuccess;
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
