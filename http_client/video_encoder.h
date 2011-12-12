// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef HTTP_CLIENT_VIDEO_CAPTURE_H_
#define HTTP_CLIENT_VIDEO_CAPTURE_H_

#include <queue>

#include "boost/scoped_array.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "http_client/basictypes.h"
#include "http_client/http_client_base.h"

namespace webmlive {

// Storage class for raw video frames. Supports only I420.
// TODO(tomfinegan): add support for conversion from common types to I420.
class VideoFrame {
 public:
  enum {
    kNoMemory = -2,
    kInvalidArg = -1,
    kSuccess = 0,
  };
  VideoFrame();
  ~VideoFrame();
  // Allocates storage for |ptr_data|, sets internal fields to values of
  // caller's args, and returns |kSuccess|.
  int32 InitI420(int32 width, int32 height, int64 timestamp, uint8* ptr_data,
                 int32 data_length);
  // Swaps |VideoFrame| member data with |ptr_frame|'s. The |VideoFrame|s
  // must be of identical size, and each must have a non-NULL buffer.
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

// Queue object used to pass video frames between threads. Uses two
// |std::queue<VideoFrame*>|s and moves |VideoFrame| pointers between them to
// provide a means by which the capture thread can pass samples to the video
// encoder.
class VideoFrameQueue {
 public:
  enum {
    // |Push| called before |Init|.
    kNoBuffers = -5,
    // No |VideoFrame|s waiting in |active_frames_|.
    kEmpty = -4,
    // No |VideoFrame|s available in |frame_pool_|.
    kFull = -3,
    kNoMemory = -2,
    kInvalidArg = -1,
    kSuccess = 0,
  };
  // Number of |VideoFrame|'s to allocate and push into the |frame_pool_|.
  static const int32 kMaxDepth = 4;
  VideoFrameQueue();
  ~VideoFrameQueue();
  // Allocates |kMaxDepth| |VideoFrame|s, pushes them into |frame_pool_|, and
  // returns |kSuccess|.
  int32 Init();
  // Grabs a |VideoFrame| from |frame_pool_|, copies the data from |ptr_frame|,
  // and pushes it into |active_frames_|. Returns |kSuccess| if able to store
  // the frame. Returns |kFull| when |frame_pool_| is empty.
  int32 Push(VideoFrame* ptr_frame);
  // Grabs a |VideoFrame| from |active_frames_| and copies it to |ptr_frame|.
  // Returns |kSuccess| when able to copy the frame. Returns |kEmpty| when
  // |active_frames_| contains no |VideoFrame|s.
  int32 Pop(VideoFrame* ptr_frame);
  // Drops all queued |VideoFrame|s by moving them all from |active_frames_| to
  // |frame_pool_|.
  void DropFrames();
  // Copies |ptr_source| to |ptr_target| using |VideoFrame::Init| or
  // |VideoFrame::Swap| based on presence of non-NULL buffer pointer in
  // |ptr_target|.
  static int32 CopyFrame(VideoFrame* ptr_source, VideoFrame* ptr_target);
 private:
  boost::mutex mutex_;
  std::queue<VideoFrame*> frame_pool_;
  std::queue<VideoFrame*> active_frames_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoFrameQueue);
};

class VideoFrameCallbackInterface {
 public:
  enum {
   kInvalidArg = -1,
   kSuccess = 0,
   kDropped = 1,
  };
  virtual int32 OnVideoFrameReceived(VideoFrame* ptr_frame) = 0;
};

struct VpxConfig {
  // Time between keyframes, in seconds.
  double keyframe_interval;
  // Video bitrate, in kilobits.
  int bitrate;
  // Video frame rate decimation factor.
  int decimate;
  // Minimum quantizer value.
  int min_quantizer;
  // Maxium quantizer value.
  int max_quantizer;
  // Encoder complexity.
  int speed;
  // Threshold at which a macroblock is considered static.
  int static_threshold;
  // Encoder thead count.
  int thread_count;
  // Number of token partitions.
  int token_partitions;
  // Percentage to undershoot the requested datarate.
  int undershoot;
};

// Forward declaration of |VpxEncoder| class for use in |VideoEncoder|. The
// libvpx implementation details are kept hidden because use of the includes
// produces C4505 warnings with MSVC at warning level 4.
class VpxEncoder;
struct WebmEncoderConfig;

class VideoEncoder {
 public:
  enum {
    kCodecError = -100,
    kNoMemory = -2,
    kInvalidArg = -1,
    kSuccess = 0,
  };
  VideoEncoder();
  ~VideoEncoder();
  int32 Init(const WebmEncoderConfig* ptr_config);
  int32 EncodeFrame(const VideoFrame* ptr_frame);
 private:
  boost::scoped_ptr<VpxEncoder> ptr_vpx_encoder_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoEncoder);
};

}  // namespace webmlive

#endif  // HTTP_CLIENT_VIDEO_CAPTURE_H_
