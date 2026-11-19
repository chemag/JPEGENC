// SPDX-License-Identifier: Apache-2.0
//
// Encode fuzzer-chosen pixels with fuzzer-chosen parameters. The first six
// input bytes select the image and the encoder path; the rest are pixels.
//
//   byte 0, 1: width - 1, height - 1 (1..256 each)
//   byte 2:    pixel type, modulo JPEGE_PIXEL_COUNT
//   byte 3:    bit 0 4:2:0 subsampling, bits 1-2 quality, bits 3-4 how many
//              bytes the input is misaligned by
//   byte 4:    bits 0-3 row padding in bytes, bit 4 AddMCU() instead of
//              AddFrame()
//   byte 5:    bit 0 callback (file) output instead of a RAM buffer, bit 1 a
//              small RAM buffer of 1024 + 64 * (byte 5 >> 2) bytes
//
// Every input plane is allocated to its exact size, so reads past the last
// row or column reach AddressSanitizer redzones.

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "jpegenc_test.h"

namespace {

constexpr size_t kHeaderSize = 6;

// Collects callback output; the encoder must never pass a negative length.
std::vector<uint8_t>* g_sink = nullptr;

void* OpenSink(const char*) { return g_sink; }
void CloseSink(JPEGE_FILE*) {}
int32_t ReadSink(JPEGE_FILE*, uint8_t*, int32_t) { return 0; }
int32_t WriteSink(JPEGE_FILE*, uint8_t* buffer, int32_t length) {
  if (length < 0 || (length > 0 && buffer == nullptr)) std::abort();
  g_sink->insert(g_sink->end(), buffer, buffer + length);
  return length;
}
int32_t SeekSink(JPEGE_FILE*, int32_t position) { return position; }

// An exactly sized input plane starting misalign bytes into its storage.
class Plane {
 public:
  Plane(size_t size, size_t misalign, const uint8_t* fill, size_t fill_size)
      : storage_(size + misalign), data_(storage_.data() + misalign) {
    for (size_t i = 0; i < size; ++i) {
      data_[i] =
          fill_size > 0 ? fill[i % fill_size] : static_cast<uint8_t>(i * 37);
    }
  }
  uint8_t* data() { return data_; }

 private:
  std::vector<uint8_t> storage_;
  uint8_t* data_;
};

int BytesPerPixel(int pixel_type) {
  switch (pixel_type) {
    case JPEGE_PIXEL_RGB565_LE:
    case JPEGE_PIXEL_RGB565_BE:
    case JPEGE_PIXEL_YUV422:
      return 2;
    case JPEGE_PIXEL_RGB888:
      return 3;
    case JPEGE_PIXEL_ARGB8888:
      return 4;
    default:
      return 1;
  }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < kHeaderSize) return 0;
  const int width = 1 + data[0];
  const int height = 1 + data[1];
  const int pixel_type = data[2] % JPEGE_PIXEL_COUNT;
  const int subsample =
      (data[3] & 1) ? JPEGE_SUBSAMPLE_420 : JPEGE_SUBSAMPLE_444;
  const int quality = (data[3] >> 1) & 3;
  const size_t misalign = (data[3] >> 3) & 3;
  const int padding = data[4] & 15;
  const bool add_mcu = (data[4] & 16) != 0;
  const bool file_output = (data[5] & 1) != 0;
  const uint8_t* fill = data + kHeaderSize;
  const size_t fill_size = size - kHeaderSize;

  std::vector<uint8_t> sink;
  std::vector<uint8_t> output;
  JPEGE_IMAGE jpeg;
  JPEGENCODE encode;
  int rc;
  if (file_output) {
    g_sink = &sink;
    rc = JPEGOpen(&jpeg, "fuzz", OpenSink, CloseSink, ReadSink, WriteSink,
                      SeekSink);
  } else {
    // A large buffer holds any encode; a small one exercises JPEGE_NO_BUFFER.
    output.resize((data[5] & 2)
                      ? 1024 + 64 * (data[5] >> 2)
                      : 65536 + static_cast<size_t>(width) * height * 16);
    rc = JPEGOpenRAM(&jpeg, output.data(), static_cast<int>(output.size()));
  }
  if (rc != JPEGE_SUCCESS) return 0;
  rc = JPEGEncodeBegin(
      &jpeg, &encode, width, height, static_cast<uint8_t>(pixel_type),
      static_cast<uint8_t>(subsample), static_cast<uint8_t>(quality));
  if (rc != JPEGE_SUCCESS) {
    (void)JPEGClose(&jpeg);
    return 0;
  }

  const int chroma_width = (width + 1) / 2;
  const int chroma_height = (height + 1) / 2;
  if (pixel_type == JPEGE_PIXEL_YUV420P || pixel_type == JPEGE_PIXEL_NV12) {
    const int y_pitch = width + padding;
    Plane y(static_cast<size_t>(y_pitch) * (height - 1) + width, misalign, fill,
            fill_size);
    if (pixel_type == JPEGE_PIXEL_YUV420P) {
      const int c_pitch = chroma_width + padding;
      const size_t c_size =
          static_cast<size_t>(c_pitch) * (chroma_height - 1) + chroma_width;
      Plane cb(c_size, misalign, fill, fill_size);
      Plane cr(c_size, misalign, fill, fill_size);
      (void)JPEGAddFrameYUV420P(&jpeg, &encode, y.data(), y_pitch,
                                    cb.data(), c_pitch, cr.data(), c_pitch);
    } else {
      const int uv_pitch = chroma_width * 2 + padding;
      Plane uv(static_cast<size_t>(uv_pitch) * (chroma_height - 1) +
                   chroma_width * 2,
               misalign, fill, fill_size);
      (void)JPEGAddFrameNV12(&jpeg, &encode, y.data(), y_pitch, uv.data(),
                                 uv_pitch);
    }
  } else {
    // Packed 4:2:2 rows hold whole YUYV pairs.
    const int row_pixels =
        pixel_type == JPEGE_PIXEL_YUV422 ? (width + 1) & ~1 : width;
    const int row_bytes = row_pixels * BytesPerPixel(pixel_type);
    const int pitch = row_bytes + padding;
    Plane pixels(static_cast<size_t>(pitch) * (height - 1) + row_bytes,
                 misalign, fill, fill_size);
    if (add_mcu) {
      const int mcu = subsample == JPEGE_SUBSAMPLE_420 ? 16 : 8;
      rc = JPEGE_SUCCESS;
      for (int my = 0; my < height && rc == JPEGE_SUCCESS; my += mcu) {
        for (int mx = 0; mx < width && rc == JPEGE_SUCCESS; mx += mcu) {
          rc = JPEGAddMCU(
              &jpeg, &encode,
              pixels.data() + static_cast<size_t>(my) * pitch +
                  static_cast<size_t>(mx) * BytesPerPixel(pixel_type),
              pitch);
        }
      }
    } else {
      (void)JPEGAddFrame(&jpeg, &encode, pixels.data(), pitch);
    }
  }
  (void)JPEGClose(&jpeg);
  (void)JPEGGetLastError(&jpeg);
  g_sink = nullptr;
  return 0;
}
