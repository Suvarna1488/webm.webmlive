// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef HTTP_CLIENT_VIDEO_CAPTURE_H_
#define HTTP_CLIENT_VIDEO_CAPTURE_H_

#include "boost/scoped_array.hpp"
#include "http_client/basictypes.h"
#include "http_client/http_client_base.h"
#include "http_client/video_types.h"

namespace webmlive {

class VideoFrame {
 public:
  enum {
    kNoMemory = -2,
    kInvalidArg = -1,
    kSuccess = 0,
  };
  VideoFrame();
  ~VideoFrame();
  // Allocates storage for |ptr_data|, sets internal fields to values in 
  // callers args, and returns |kSuccess|.
  int32 InitI420(int32 width, int32 height, int64 timestamp, uint8* ptr_data, 
                 int32 data_length);
  // Swaps |VideoFrame| data.
  void Swap(VideoFrame* ptr_frame);
  int32 width() const { return width_; }
  int32 height() const { return height_; }
  int64 timestamp() const { return timestamp_; }
  uint8* buffer() const { return buffer_.get(); }
  int32 buffer_length() const { return buffer_length_; }
 private:
  int32 width_;
  int32 height_;
  int64 timestamp_;
  boost::scoped_array<uint8> buffer_;
  int32 buffer_length_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoFrame);
};

class VideoFrameCallbackInterface {
 public:
  enum {
   kInvalidArg = -2,
   kNullFrame = -1,
   kSuccess = 0,
   kDropped = 1,
  };
  virtual int32 OnVideoFrameReceived(VideoFrame* ptr_frame) = 0;
};

}  // namespace webmlive

#endif  // HTTP_CLIENT_VIDEO_CAPTURE_H_
