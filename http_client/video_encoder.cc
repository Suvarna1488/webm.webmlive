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

#if defined _MSC_VER
// Disable warning C4505(unreferenced local function has been removed) in MSVC.
// At the time this comment was written the warning is emitted 27 times for
// vp8.h and vp8cx.h (included by vpx_encoder.h).
#pragma warning(disable:4505)
#endif
#include "http_client/vpx_encoder.h"

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

///////////////////////////////////////////////////////////////////////////////
// VideoFrameQueue
//

VideoFrameQueue::VideoFrameQueue() {
}

VideoFrameQueue::~VideoFrameQueue() {
  boost::mutex::scoped_lock lock(mutex_);
  while (!frame_pool_.empty()) {
    delete frame_pool_.front();
    frame_pool_.pop();
  }
  while (!active_frames_.empty()) {
    delete active_frames_.front();
    active_frames_.pop();
  }
}

// Obtains lock and populates |frame_pool_| with |VideoFrame| pointers.
int32 VideoFrameQueue::Init() {
  boost::mutex::scoped_lock lock(mutex_);
  DCHECK(frame_pool_.empty());
  DCHECK(active_frames_.empty());
  for (int i = 0; i < kMaxDepth; ++i) {
    VideoFrame* ptr_frame = new (std::nothrow) VideoFrame;
    if (!ptr_frame) {
      LOG(ERROR) << "VideoFrame allocation failed!";
      return kNoMemory;
    }
    frame_pool_.push(ptr_frame);
  }
  return kSuccess;
}

// Obtains lock, copies |ptr_frame| data into |VideoFrame| from |frame_pool_|,
// and moves the frame into |active_frames_|.
int32 VideoFrameQueue::Push(VideoFrame* ptr_frame) {
  if (!ptr_frame || !ptr_frame->buffer()) {
    LOG(ERROR) << "VideoFrameQueue can't Push a NULL/empty VideoFrame!";
    return kInvalidArg;
  }
  boost::mutex::scoped_lock lock(mutex_);
  if (frame_pool_.empty()) {
    VLOG(4) << "VideoFrameQueue full.";
    return kFull;
  }
  // Copy user data into front frame from |frame_pool_|.
  VideoFrame* ptr_pool_frame = frame_pool_.front();
  if (CopyFrame(ptr_frame, ptr_pool_frame)) {
    LOG(ERROR) << "VideoFrame CopyFrame failed!";
    return kNoMemory;
  }
  // Move the now active frame from the pool into the active queue.
  frame_pool_.pop();
  active_frames_.push(ptr_pool_frame);
  return kSuccess;
}

// Obtains lock, copies front |VideoFrame| from |active_frames_| to
// |ptr_frame|, and moves the consumed |VideoFrame| back into |frame_pool_|.
int32 VideoFrameQueue::Pop(VideoFrame* ptr_frame) {
  if (!ptr_frame) {
    LOG(ERROR) << "VideoFrameQueue can't Pop into a NULL VideoFrame!";
    return kInvalidArg;
  }
  boost::mutex::scoped_lock lock(mutex_);
  if (active_frames_.empty()) {
    VLOG(4) << "VideoFrameQueue empty.";
    return kEmpty;
  }
  // Copy active frame data to user frame.
  VideoFrame* ptr_active_frame = active_frames_.front();
  if (CopyFrame(ptr_active_frame, ptr_frame)) {
    LOG(ERROR) << "CopyFrame failed!";
    return kNoMemory;
  }
  // Put the now inactive frame back in the pool.
  active_frames_.pop();
  frame_pool_.push(ptr_active_frame);
  return kSuccess;
}

// Obtains lock and drops any |VideoFrame|s in |active_frames_|.
void VideoFrameQueue::DropFrames() {
  boost::mutex::scoped_lock lock(mutex_);
  while (!active_frames_.empty()) {
    frame_pool_.push(active_frames_.front());
    active_frames_.pop();
  }
}

int32 VideoFrameQueue::CopyFrame(VideoFrame* ptr_source,
                                 VideoFrame* ptr_target) {
  if (!ptr_source || !ptr_target) {
    return kInvalidArg;
  }
  if (ptr_target->buffer()) {
    ptr_target->Swap(ptr_source);
  } else {
    int status = ptr_target->InitI420(ptr_source->width(),
                                      ptr_source->height(),
                                      ptr_source->timestamp(),
                                      ptr_source->buffer(),
                                      ptr_source->buffer_length());
    if (status) {
      LOG(ERROR) << "VideoFrame Init failed! " << status;
      return kNoMemory;
    }
  }
  return kSuccess;
}

///////////////////////////////////////////////////////////////////////////////
// VideoEncoder
//

VideoEncoder::VideoEncoder() {
}

VideoEncoder::~VideoEncoder() {
}

int32 VideoEncoder::Init(const WebmEncoderConfig* ptr_config) {
  ptr_vpx_encoder_.reset(new (std::nothrow) VpxEncoder());
  if (!ptr_vpx_encoder_) {
    return kNoMemory;
  }
  return ptr_vpx_encoder_->Init(ptr_config);
}

}  // namespace webmlive
