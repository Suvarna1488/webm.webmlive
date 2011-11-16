// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef HTTP_CLIENT_VIDEO_CAPTURE_H_
#define HTTP_CLIENT_VIDEO_CAPTURE_H_

#include "http_client/basictypes.h"
#include "http_client/video_types.h"

namespace webmlive {

class VideoFrame {
 public:
  VideoFrame();
  ~VideoFrame();

 private:
  I420Frame i420_frame;
  int64 timestamp;
  uint8* ptr_data;
  int32 data_length;
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

#endif  // HTTP_CLIENT_VIDEO_CAPTURE_H_
