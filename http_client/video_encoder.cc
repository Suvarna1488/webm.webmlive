// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "http_client/video_encoder.h"

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

}  // namespace webmlive
