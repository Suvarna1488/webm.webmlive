// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

// Defines the video sink filter used to obtain raw frames from user input
// devices available via DirectShow. Based on WebRTC's CaptureInputPin and
// CaptureSinkFilter.

#ifndef HTTP_CLIENT_WIN_VIDEO_SINK_FILTER_H_
#define HTTP_CLIENT_WIN_VIDEO_SINK_FILTER_H_

#include "baseclasses/streams.h"
#include "boost/scoped_array.hpp"
#include "boost/scoped_ptr.hpp"
#include "http_client/basictypes.h"
#include "http_client/http_client_base.h"
#include "http_client/video_encoder.h"
#include "http_client/video_types.h"
#include "http_client/webm_encoder.h"

namespace webmlive {

// Forward declare |VideoSinkFilter| for use in |VideoSinkPin|.
class VideoSinkFilter;

class VideoSinkPin : public CBaseInputPin {
 public:
  typedef WebmEncoderConfig::VideoCaptureConfig VideoConfig;
  VideoSinkPin(TCHAR* ptr_object_name,
               VideoSinkFilter* ptr_filter,
               CCritSec* ptr_filter_lock,
               HRESULT* ptr_result,
               LPCWSTR ptr_pin_name);
  virtual ~VideoSinkPin();
  //
  // CBaseInputPin methods
  //
  // Stores preferred media type for |type_index| in |ptr_media_type|. Supports
  // only a single type, I420.
  // Return values:
  // |S_OK| - success, |type_index| in range and |ptr_media_type| written.
  // |VFW_S_NO_MORE_ITEMS| - |type_index| != 0.
  // |E_OUTOFMEMORY| - could not allocate format buffer.
  HRESULT GetMediaType(int32 type_index, CMediaType* ptr_media_type);
  HRESULT CheckMediaType(const CMediaType* ptr_media_type);
  // IPin method(s).
  STDMETHODIMP Receive(IMediaSample* ptr_sample);
  
 private:
  // Writes |actual_config_| values to |ptr_config| and returns |S_OK|.
  HRESULT config(VideoConfig* ptr_config);
  // Resets |actual_config_| and copies |config| values to |requested_config_|,
  // then returns |S_OK|.
  HRESULT set_config(const VideoConfig& config);
  // Filter user's requested video config.
  VideoConfig requested_config_;
  // Actual video config (from source filter).
  VideoConfig actual_config_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoSinkPin);
  friend class VideoSinkFilter;
};

class VideoSinkFilter : public CBaseFilter {
 public:
  typedef WebmEncoderConfig::VideoCaptureConfig VideoConfig;
  VideoSinkFilter(TCHAR* ptr_filter_name,
                  LPUNKNOWN ptr_iunknown,
                  VideoFrameCallback* ptr_frame_callback,
                  HRESULT* ptr_result);
  virtual ~VideoSinkFilter();
  HRESULT config(VideoConfig* ptr_config);
  HRESULT set_config(const VideoConfig& config);
  // IUnknown
  DECLARE_IUNKNOWN;
  // CBaseFilter methods
  int GetPinCount() { return 1; }
  CBasePin* GetPin(int index);

 private:
  HRESULT OnFrameReceived(IMediaSample* ptr_sample);
  CCritSec filter_lock_;
  int32 frame_buffer_length_;
  boost::scoped_array<uint8> frame_buffer_;
  boost::scoped_ptr<VideoSinkPin> sink_pin_;
  VideoFrameCallback* ptr_frame_callback_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoSinkFilter);
  friend class VideoSinkPin;
};

}  // namespace webmlive

#endif  // HTTP_CLIENT_WIN_VIDEO_SINK_FILTER_H_
