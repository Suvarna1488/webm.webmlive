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
    kNoMemory = -2,
    kInvalidArg = -1,
    kSuccess = 0,
  };
  VpxEncoder();
  ~VpxEncoder();
  int Init(const WebmEncoderConfig* ptr_config);
  int EncodeFrame(const VideoFrame* ptr_frame);
 private:
  vpx_codec_ctx_t vp8_context_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VpxEncoder);
};

}  // namespace webmlive

#endif
