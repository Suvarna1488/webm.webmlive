// Copyright (c) 2011 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "http_client_base.h"

#include <conio.h>
#include <stdio.h>
#include <tchar.h>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "boost/scoped_array.hpp"
#include "boost/thread/thread.hpp"

#include "debug_util.h"
#include "buffer_util.h"
#include "file_reader.h"
#include "http_uploader.h"
#include "webm_encoder.h"

namespace {
  const double kDefaultKeyframeInterval = 2.0;
}  // anonymous namespace

void usage(const wchar_t** argv) {
  printf("Usage: %ls --file <local file name> --url <target URL>\n", argv[0]);
  printf("  Note: file and url params are required.\n");
  printf("Options:\n");
  printf("  -h | -? | --help               Show this message and exit.\n");
  printf("  --keyframe_interval <seconds>  Time between keyframes.\n");
}

int store_string_map_entries(const std::vector<std::string>& unparsed_entries,
                             std::map<std::string, std::string>& out_map)
{
  using std::string;
  using std::vector;
  vector<string>::const_iterator entry_iter = unparsed_entries.begin();
  while (entry_iter != unparsed_entries.end()) {
    // TODO(tomfinegan): support empty headers?
    const string& entry = *entry_iter;
    size_t sep = entry.find(":");
    if (sep == string::npos) {
      // bad header (missing separator, no value)
      // TODO(tomfinegan): allow empty entries?
      DBGLOG("ERROR: cannot parse entry, should be name:value, got="
             << entry.c_str());
      return ERROR_BAD_FORMAT;
    }
    out_map[entry.substr(0, sep).c_str()] = entry.substr(sep+1);
    ++entry_iter;
  }
  return ERROR_SUCCESS;
}

void parse_command_line(int argc, const wchar_t* argv[],
                        webmlive::HttpUploaderSettings& uploader_settings,
                        webmlive::WebmEncoderSettings& encoder_settings) {

  typedef std::vector<std::wstring> WStringVector;
  WStringVector unparsed_widechar_headers;
  WStringVector unparsed_widechar_vars;
  encoder_settings.keyframe_interval = kDefaultKeyframeInterval;
  for (int i = 1; i < argc; ++i) {
    if (!wcscmp(L"-h", argv[i]) || !wcscmp(L"-?", argv[i]) ||
        !wcscmp(L"--help", argv[i])) {
      usage(argv);
      exit(EXIT_SUCCESS);
    } else if (!wcscmp(L"--file", argv[i])) {
      std::ostringstream str;
      str << std::wstring(argv[++i]).c_str();
      uploader_settings.local_file = str.str();
      encoder_settings.output_file_name = uploader_settings.local_file;
    } else if (!wcscmp(L"--url", argv[i])) {
      std::ostringstream str;
      str << *argv[++i];
      uploader_settings.target_url = str.str();
    } else if (!wcscmp(L"--header", argv[i])) {
      unparsed_widechar_headers.push_back(argv[++i]);
    } else if (!wcscmp(L"--var", argv[i])) {
      unparsed_widechar_vars.push_back(argv[++i]);
    } else if (!wcscmp(L"--keyframe_interval", argv[i])) {
      wchar_t* ptr_end;
      encoder_settings.keyframe_interval = wcstod(argv[++i], &ptr_end);
    }
  }
  // Store user HTTP headers.
  typedef std::vector<std::string> StringVector;
  StringVector unparsed_headers;
  WStringVector::const_iterator i = unparsed_widechar_headers.begin();
  for (; i != unparsed_widechar_headers.end(); ++i) {
    std::ostringstream str;
    str << (*i).c_str();//unparsed_widechar_headers[i];
    unparsed_headers.push_back(str.str());
  }
  store_string_map_entries(unparsed_headers, uploader_settings.headers);
  // Store user form variables.
  StringVector unparsed_vars;
  i = unparsed_widechar_vars.begin();
  for (; i != unparsed_widechar_vars.end(); ++i) {
    std::ostringstream str;
    str << (*i).c_str();//unparsed_widechar_vars[i];
    unparsed_vars.push_back(str.str());
  }
  store_string_map_entries(unparsed_vars, uploader_settings.form_variables);
}


// Calls |Init| and |Run| on |encoder| to start the encode of a WebM file.
// TODO(tomfinegan): Add capture and encoder settings configuration.
int start_encoder(webmlive::WebmEncoder& encoder,
                  const webmlive::WebmEncoderSettings& settings) {
  int status = encoder.Init(settings);
  if (status) {
    DBGLOG("encoder Init failed, status=" << status);
    return status;
  }
  status = encoder.Run();
  if (status) {
    DBGLOG("encoder Run failed, status=" << status);
  }
  return status;
}

// Calls |Init| and |Run| on |uploader| to start the uploader thread, which
// uploads buffers when |UploadBuffer| is called on the uploader.
int start_uploader(webmlive::HttpUploader& uploader,
                   webmlive::HttpUploaderSettings& settings) {
  int status = uploader.Init(settings);
  if (status) {
    DBGLOG("uploader Init failed, status=" << status);
    return status;
  }
  status = uploader.Run();
  if (status) {
    DBGLOG("uploader Run failed, status=" << status);
  }
  return status;
}

int client_main(webmlive::HttpUploaderSettings& uploader_settings,
                const webmlive::WebmEncoderSettings& encoder_settings) {
  // Setup the file reader.  This is a little strange since |reader| actually
  // creates the output file that is used by the encoder.
  webmlive::FileReader reader;
  int status = reader.CreateFile(uploader_settings.local_file);
  if (status) {
    fprintf(stderr, "file reader init failed, status=%d.\n", status);
    return EXIT_FAILURE;
  }
  // Start encoding the WebM file.
  webmlive::WebmEncoder encoder;
  status = start_encoder(encoder, encoder_settings);
  if (status) {
    fprintf(stderr, "start_encoder failed, status=%d\n", status);
    return EXIT_FAILURE;
  }
  // Start the uploader thread.
  webmlive::HttpUploader uploader;
  status = start_uploader(uploader, uploader_settings);
  if (status) {
    fprintf(stderr, "start_uploader failed, status=%d\n", status);
    encoder.Stop();
    return EXIT_FAILURE;
  }
  webmlive::HttpUploaderStats stats;
  const int32 kReadBufferSize = 100*1024;
  int32 read_buffer_size = kReadBufferSize;
  using boost::scoped_array;
  scoped_array<uint8> read_buf(new (std::nothrow) uint8[kReadBufferSize]);
  if (!read_buf) {
    uploader.Stop();
    encoder.Stop();
    fprintf(stderr, "out of memory, can't alloc read_buf.\n");
    return EXIT_FAILURE;
  }
  webmlive::WebmChunkBuffer chunk_buffer;
  status = chunk_buffer.Init();
  if (status) {
    uploader.Stop();
    encoder.Stop();
    fprintf(stderr, "can't create chunk buffer.\n");
    return EXIT_FAILURE;
  }
  // Loop until the user hits a key.
  int exit_code = EXIT_SUCCESS;
  printf("\nPress the any key to quit...\n");
  while(!_kbhit()) {
    // Output current duration and upload progress
    if (uploader.GetStats(&stats) == webmlive::HttpUploader::kSuccess) {
      printf("\rencoded duration: %04f seconds, uploaded: %I64d @ %d kBps",
             encoder.encoded_duration(),
             stats.bytes_sent_current + stats.total_bytes_uploaded,
             static_cast<int>(stats.bytes_per_second / 1000));
    }
    size_t bytes_read = 0;
    status = reader.Read(read_buffer_size, &read_buf[0], &bytes_read);
    if (bytes_read > 0) {
      status = chunk_buffer.BufferData(&read_buf[0],
                                       static_cast<int32>(bytes_read));
      if (status) {
        DBGLOG("BufferData failed, status=" << status);
        fprintf(stderr, "\nERROR: cannot add to chunk buffer!\n");
        break;
      }
    }
    if (uploader.UploadComplete()) {
      int32 chunk_length = 0;
      if (chunk_buffer.ChunkReady(&chunk_length)) {
        if (chunk_length > read_buffer_size) {
          // Reallocate the read buffer-- the chunk is too large.
          read_buf.reset(new (std::nothrow) uint8[chunk_length]);
          if (!read_buf) {
            DBGLOG("read buffer reallocation failed");
            fprintf(stderr, "\nERROR: cannot reallocate read buffer!\n");
            exit_code = EXIT_FAILURE;
            break;
          }
          read_buffer_size = chunk_length;
        }
        status = chunk_buffer.ReadChunk(&read_buf[0], chunk_length);
        if (status) {
          DBGLOG("ReadChunk failed, status=" << status);
          fprintf(stderr, "\nERROR: cannot read chunk!\n");
          exit_code = EXIT_FAILURE;
          break;
        }
        // Start upload of the read buffer contents
        DBGLOG("starting buffer upload, chunk_length=" << chunk_length);
        status = uploader.UploadBuffer(&read_buf[0], chunk_length);
        if (status) {
          DBGLOG("UploadBuffer failed, status=" << status);
          fprintf(stderr, "\nERROR: can't upload buffer!\n");
          exit_code = EXIT_FAILURE;
          break;
        }
      }
    }
    Sleep(100);
  }
  DBGLOG("stopping encoder...");
  encoder.Stop();
  DBGLOG("stopping uploader...");
  uploader.Stop();
  printf("\nDone.\n");
  return exit_code;
}

int _tmain(int argc, const wchar_t** argv) {
  webmlive::HttpUploaderSettings uploader_settings;
  webmlive::WebmEncoderSettings encoder_settings;
  parse_command_line(argc, argv, uploader_settings, encoder_settings);
  // validate params
  // TODO(tomfinegan): can probably make program options enforce this for me...
  if (uploader_settings.target_url.empty() ||
      encoder_settings.output_file_name.empty()) {
    fprintf(stderr, "file and url params are required!\n");
    usage(argv);
    return EXIT_FAILURE;
  }
  DBGLOG("file: " << encoder_settings.output_file_name.c_str());
  DBGLOG("url: " << uploader_settings.target_url.c_str());
  return client_main(uploader_settings, encoder_settings);
}

// We build with BOOST_NO_EXCEPTIONS defined; boost will call this function
// instead of throwing.  We must stop execution here.
void boost::throw_exception(const std::exception& e) {
  fprintf(stderr, "Fatal error: %s\n", e.what());
  exit(EXIT_FAILURE);
}
