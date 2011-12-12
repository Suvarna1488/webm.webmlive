// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef HTTP_CLIENT_VPX_ENCODER_H_
#define HTTP_CLIENT_VPX_ENCODER_H_

#include "http_client/basictypes.h"
#include "http_client/http_client_base.h"
#include "http_client/video_encoder.h"
#include "libvpx/vpx/vpx_encoder.h"
#include "libvpx/vpx/vp8cx.h"

namespace webmlive {
class VideoFrame;
struct WebmEncoderConfig;

class VpxEncoder {
 public:
  enum {
    kCodecError = VideoEncoder::kCodecError,
    kEncoderError = VideoEncoder::kEncoderError,
    kNoMemory = VideoEncoder::kNoMemory,
    kInvalidArg = VideoEncoder::kInvalidArg,
    kSuccess = VideoEncoder::kSuccess,
    kDropped = VideoEncoder::kDropped,
  };
  VpxEncoder();
  ~VpxEncoder();
  int32 Init(const WebmEncoderConfig* ptr_config);
  int32 EncodeFrame(const VideoFrame* const ptr_raw_frame, 
                    VideoFrame* ptr_vp8_frame);
 private:
  int64 frames_in_;
  int64 frames_out_;
  int64 last_keyframe_time_;
  VpxConfig config_;
  vpx_codec_ctx_t vp8_context_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VpxEncoder);
};

}  // namespace webmlive

#endif
