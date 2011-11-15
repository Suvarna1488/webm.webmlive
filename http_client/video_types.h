// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

// Types in this file based on those found in the webrtc video_capture module.

#ifndef HTTP_CLIENT_VIDEO_TYPES_H_
#define HTTP_CLIENT_VIDEO_TYPES_H_

#include "http_client/basictypes.h"

namespace webmlive {

enum VideoFormat {
  kI420 = 0,
  kYV12 = 1,
  kYUY2 = 2,
  kYUYV = 3,
  kV210 = 4,
  kNumVideoSubtypes = 5,
};

static const int kI420BitCount = 12;
static const int kYV12BitCount = kI420BitCount;
static const int kYUY2BitCount = 16;
static const int kYUYVBitCount = kYUY2BitCount;
static const int kV210BitCount = 32;

struct I420Frame {
  I420Frame() {
    ptr_y_plane = NULL;
    ptr_u_plane = NULL;
    ptr_v_plane = NULL;
    y_pitch = 0;
    u_pitch = 0;
    v_pitch = 0;
    width = 0;
    height = 0;
  }

  uint8* ptr_y_plane;
  uint8* ptr_u_plane;
  uint8* ptr_v_plane;

  int32 y_pitch;
  int32 u_pitch;
  int32 v_pitch;

  int32 width;
  int32 height;
};

struct VideoFrame {
  VideoFrame() {
    timestamp = 0;
  }
  I420Frame i420_frame;
  int64 timestamp;
};

class VideoFrameCallback {
 enum {
   kSuccess = 0,
   kDropped = 1,
 };
 public:
  virtual int OnVideoFrameReceived(VideoFrame* ptr_frame) = 0;
};

}  // namespace webmlive

#endif  // HTTP_CLIENT_VIDEO_TYPES_H_
