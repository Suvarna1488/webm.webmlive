// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "http_client/win/video_sink_filter.h"

#include <dvdmedia.h>
#include <vfwmsgs.h>

#include "glog/logging.h"
#include "http_client/video_types.h"
#include "http_client/win/webm_guids.h"

// TODO(tomfinegan): webrtc uses baseclasses, but has worked around the need
//                   for the next two lines. Determining how to do so would be
//                   enlightening, but isn't that important.
//                   Without these two lines dllentry.cpp from the baseclasses
//                   sources will cause an error at link time (LNK2001,
//                   unresolved external symbol) because of use of the following
//                   two globals via extern.
CFactoryTemplate* g_Templates = NULL;   // NOLINT
int g_cTemplates = 0;                   // NOLINT

namespace webmlive {

///////////////////////////////////////////////////////////////////////////////
// VideoSinkPin
//

VideoSinkPin::VideoSinkPin(TCHAR* ptr_object_name,
                           VideoSinkFilter* ptr_filter,
                           CCritSec* ptr_filter_lock,
                           HRESULT* ptr_result,
                           LPCWSTR ptr_pin_name)
    : CBaseInputPin(ptr_object_name, ptr_filter, ptr_filter_lock, ptr_result,
                    ptr_pin_name) {
}

VideoSinkPin::~VideoSinkPin() {
}

HRESULT VideoSinkPin::GetMediaType(int32 type_index,
                                   CMediaType* ptr_media_type) {
  // TODO: add libyuv and support types other than I420
  if (type_index != 0) {
    return VFW_S_NO_MORE_ITEMS;
  }
  VIDEOINFOHEADER* const ptr_video_info =
      reinterpret_cast<VIDEOINFOHEADER*>(
          ptr_media_type->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
  if (!ptr_video_info) {
    LOG(ERROR) << "VIDEOINFOHEADER alloc failed.";
    return E_OUTOFMEMORY;
  }
  ZeroMemory(ptr_video_info, sizeof(VIDEOINFOHEADER));
  ptr_video_info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  // Use empty source/dest rectangle-- the entire image is needed, and there is
  // no target subrect.
  SetRectEmpty(&ptr_video_info->rcSource);
  SetRectEmpty(&ptr_video_info->rcTarget);
  // Set values for all input types supported.
  ptr_media_type->SetType(&MEDIATYPE_Video);
  ptr_media_type->SetFormatType(&FORMAT_VideoInfo);
  ptr_media_type->SetTemporalCompression(FALSE);
  ptr_video_info->bmiHeader.biWidth = requested_config_.width;
  ptr_video_info->bmiHeader.biHeight = requested_config_.height;
  // Set sub type and format data for I420.
  ptr_video_info->bmiHeader.biCompression = MAKEFOURCC('I','4','2','0');
  ptr_video_info->bmiHeader.biBitCount = kI420BitCount;
  ptr_video_info->bmiHeader.biPlanes = 1;
  ptr_media_type->SetSubtype(&MEDIASUBTYPE_I420);
  // Set sample size.
  ptr_video_info->bmiHeader.biSizeImage = DIBSIZE(ptr_video_info->bmiHeader);
  ptr_media_type->SetSampleSize(ptr_video_info->bmiHeader.biSizeImage);
  LOG(INFO) << "\n GetMediaType type_index=" << type_index << "\n"
            << "   width=" << requested_config_.width << "\n"
            << "   height=" << requested_config_.height << "\n"
            << std::hex << "   biCompression="
            << ptr_video_info->bmiHeader.biCompression;
  return S_OK;
}

HRESULT VideoSinkPin::CheckMediaType(const CMediaType* ptr_media_type) {
  // Confirm media type is acceptable.
  const GUID* const ptr_type_guid = ptr_media_type->Type();
  if (!ptr_type_guid || *ptr_type_guid != MEDIATYPE_Video) {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  // Confirm that subtype and formattype GUIDs can be obtained.
  const GUID* const ptr_subtype_guid = ptr_media_type->Subtype();
  const GUID* const ptr_format_guid = ptr_media_type->FormatType();
  if (!ptr_subtype_guid || !ptr_format_guid) {
      return E_INVALIDARG;
  }
  // Inspect the format stored in |ptr_media_type|.
  const GUID& format_guid = *ptr_format_guid;
  const GUID& subtype_guid = *ptr_subtype_guid;
  if (format_guid == FORMAT_VideoInfo) {
    const VIDEOINFOHEADER* const ptr_video_info =
        reinterpret_cast<VIDEOINFOHEADER*>(ptr_media_type->Format());
    if (!ptr_video_info) {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
    const DWORD& four_cc = ptr_video_info->bmiHeader.biCompression;
    if (subtype_guid != MEDIASUBTYPE_I420 ||
        four_cc != MAKEFOURCC('I','4','2','0')) {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
    // Store current format in |actual_config_|; |CBasePin::ReceiveConnection|
    // always calls |CheckMediaType|.
    actual_config_.width = ptr_video_info->bmiHeader.biWidth;
    actual_config_.height = abs(ptr_video_info->bmiHeader.biHeight);
  } else if (format_guid == FORMAT_VideoInfo2) {
    const VIDEOINFOHEADER2* const ptr_video_info =
        reinterpret_cast<VIDEOINFOHEADER2*>(ptr_media_type->Format());
    if (!ptr_video_info) {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
    const DWORD& four_cc = ptr_video_info->bmiHeader.biCompression;
    if (subtype_guid != MEDIASUBTYPE_I420 ||
        four_cc != MAKEFOURCC('I','4','2','0')) {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
    // Store current format in |actual_config_|; |CBasePin::ReceiveConnection|
    // always calls |CheckMediaType|.
    actual_config_.width = ptr_video_info->bmiHeader.biWidth;
    actual_config_.height = abs(ptr_video_info->bmiHeader.biHeight);
  }
  LOG(INFO) << "\n CheckMediaType actual settings\n"
            << "   width=" << requested_config_.width << "\n"
            << "   height=" << requested_config_.height;
  return S_OK;
}

HRESULT VideoSinkPin::Receive(IMediaSample* ptr_sample) {
  CHECK_NOTNULL(m_pFilter);
  CHECK_NOTNULL(ptr_sample);
  VideoSinkFilter* ptr_filter = reinterpret_cast <VideoSinkFilter*>(m_pFilter);
  CAutoLock lock(&ptr_filter->filter_lock_);
  HRESULT hr = CBaseInputPin::Receive(ptr_sample);
  if (SUCCEEDED(hr)) {
    // grab sample buffer
    // store it in |VideoFrame|
    // call VideoFrameCallback
  }
  return E_NOTIMPL;
}

// Lock always owned by caller, |VideoSinkFilter::SetConfig|.
HRESULT VideoSinkPin::SetConfig(const VideoConfig& config) {
  // TODO(tomfinegan): Sanity check values in |config|.
  requested_config_ = config;
  actual_config_ = WebmEncoderConfig::VideoCaptureConfig();
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// VideoSinkFilter
//
VideoSinkFilter::VideoSinkFilter(TCHAR* ptr_filter_name,
                                 LPUNKNOWN ptr_iunknown,
                                 HRESULT* ptr_result)
    : CBaseFilter(ptr_filter_name, ptr_iunknown, &filter_lock_,
                  CLSID_VideoSinkFilter),
      ptr_sink_pin_(NULL) {
  *ptr_result = E_FAIL;
  ptr_sink_pin_ = new VideoSinkPin(NAME("VideoSinkInputPin"), this,
                                   &filter_lock_, ptr_result, L"VideoSink");
  if (!ptr_sink_pin_ || FAILED(*ptr_result)) {
      *ptr_result = FAILED(*ptr_result) ? (*ptr_result) : E_OUTOFMEMORY;
  }
}

VideoSinkFilter::~VideoSinkFilter() {
    delete ptr_sink_pin_;
    ptr_sink_pin_ = NULL;
}

HRESULT VideoSinkFilter::SetConfig(const VideoConfig& config) {
  CAutoLock lock(&filter_lock_);
  return sink_pin_->SetConfig(config);
}

CBasePin* VideoSinkFilter::GetPin(int index) {
  CBasePin* ptr_pin = NULL;
  CAutoLock lock(&filter_lock_);
  if (index == 0) {
    ptr_pin = sink_pin_.get();
  }
  return ptr_pin;
}



}  // namespace webmlive
