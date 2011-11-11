// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "http_client/win/video_sink_filter.h"

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
  return E_NOTIMPL;
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
