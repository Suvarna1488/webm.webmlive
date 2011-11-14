// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "http_client/win/video_sink_filter.h"

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
  if (type_index > 0) {
    return VFW_S_NO_MORE_ITEMS;
  }
  VIDEOINFOHEADER* ptr_video_info =
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

  ptr_media_type->SetType(&MEDIATYPE_Video);
  ptr_media_type->SetFormatType(&FORMAT_VideoInfo);
  ptr_media_type->SetTemporalCompression(FALSE);

  ptr_video_info->bmiHeader.biWidth = requested_config_.width;
  ptr_video_info->bmiHeader.biHeight = requested_config_.height;
  ptr_video_info->bmiHeader.biCompression = MAKEFOURCC('I','4','2','0');
  ptr_video_info->bmiHeader.biBitCount = kI420BitCount;
  ptr_video_info->bmiHeader.biPlanes = 3;
  ptr_video_info->bmiHeader.biSizeImage = DIBSIZE(ptr_video_info->bmiHeader);

  ptr_media_type->SetSubtype(&MEDIASUBTYPE_I420);
  ptr_media_type->SetSampleSize(ptr_video_info->bmiHeader.biSizeImage);

  LOG(INFO) << "\n GetMediaType type_index=" << type_index << "\n"
            << "   width=" << requested_config_.width << "\n"
            << "   height=" << requested_config_.height << "\n"
            << std::hex << "   biCompression="
            << ptr_video_info->bmiHeader.biCompression;

  return S_OK;
}

HRESULT VideoSinkPin::CheckMediaType(const CMediaType* ptr_media_type) {
  return E_NOTIMPL;
}

HRESULT VideoSinkPin::Receive(IMediaSample* ptr_sample) {
  return E_NOTIMPL;
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

}  // namespace webmlive
