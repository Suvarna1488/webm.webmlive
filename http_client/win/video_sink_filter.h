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
#include "http_client/basictypes.h"
#include "http_client/http_client_base.h"
#include "http_client/webm_encoder.h"

namespace webmlive {

// Forward declare |VideoSinkFilter| for use in |VideoSinkPin|.
class VideoSinkFilter;

class VideoSinkPin : public CBaseInputPin {
 public:
  enum {
    kInvalidArg = E_INVALIDARG,
    kSuccess = S_OK,
    kFalse = S_FALSE,
  };
  VideoSinkPin(TCHAR* ptr_object_name,
               VideoSinkFilter* ptr_filter,
               CCritSec* ptr_filter_lock,
               HRESULT* ptr_result,
               LPCWSTR ptr_pin_name);
  virtual ~VideoSinkPin();

  HRESULT GetMediaType(int32 type_index, CMediaType* ptr_media_type);
  HRESULT CheckMediaType(const CMediaType* ptr_media_type);
  STDMETHODIMP Receive(IMediaSample* ptr_sample);
  HRESULT SetConfig(const WebmEncoderConfig::VideoCaptureConfig& config);
 private:
  WebmEncoderConfig::VideoCaptureConfig requested_config_;
  WebmEncoderConfig::VideoCaptureConfig actual_config_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoSinkPin);
};

class VideoSinkFilter : public CBaseFilter {
 public:
  enum {
    kInvalidArg = E_INVALIDARG,
    kSuccess = S_OK,
    kFalse = S_FALSE,
  };
  VideoSinkFilter(TCHAR* ptr_filter_name,
                  LPUNKNOWN ptr_iunknown,
                  HRESULT* ptr_result);
  virtual ~VideoSinkFilter();
 private:
  // The filter lock. Must be acquired after the receive lock.
  CCritSec filter_lock_;
  CCritSec receive_lock_;
  VideoSinkPin* ptr_sink_pin_;
  WEBMLIVE_DISALLOW_COPY_AND_ASSIGN(VideoSinkFilter);
};

}  // namespace webmlive

#endif  // HTTP_CLIENT_WIN_VIDEO_SINK_FILTER_H_
