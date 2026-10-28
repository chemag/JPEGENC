/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Close an encoder whose EncodeBegin() failed. EncodeEnd() appended the EOI
 * marker through pc.pOut, which is NULL until EncodeBegin() succeeds, so
 * callback output crashed, and RAM output wrote a stray EOI at the start of
 * the caller's buffer. Close() must write nothing and report no data. Build
 * with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/JPEGENC.h"

static int g_writes;
static int g_handle;

static void* OpenSink(const char* name) {
  (void)name;
  return &g_handle;
}
static void CloseSink(JPEGE_FILE* file) { (void)file; }
static int32_t ReadSink(JPEGE_FILE* file, uint8_t* buffer, int32_t length) {
  (void)file;
  (void)buffer;
  (void)length;
  return 0;
}
static int32_t WriteSink(JPEGE_FILE* file, uint8_t* buffer, int32_t length) {
  (void)file;
  (void)buffer;
  g_writes++;
  return length;
}
static int32_t SeekSink(JPEGE_FILE* file, int32_t position) {
  (void)file;
  return position;
}

/* Planar YUV420 requires 4:2:0, so this EncodeBegin() fails. */
static int BeginFails(JPEGE_IMAGE* jpeg) {
  JPEGENCODE encode;
  return JPEGENC_EncodeBegin(jpeg, &encode, 16, 16, JPEGE_PIXEL_YUV420P,
                             JPEGE_SUBSAMPLE_444,
                             JPEGE_Q_HIGH) == JPEGE_INVALID_PARAMETER;
}

int main(void) {
  static uint8_t output[2048], untouched[2048];
  JPEGE_IMAGE jpeg;
  int size;

  if (JPEGENC_Open(&jpeg, "sink", OpenSink, CloseSink, ReadSink, WriteSink,
                   SeekSink) != JPEGE_SUCCESS ||
      !BeginFails(&jpeg)) {
    fprintf(stderr, "callback setup failed\n");
    return 1;
  }
  size = JPEGENC_Close(&jpeg);
  if (size != 0 || g_writes != 0) {
    fprintf(stderr, "callback close: size %d, writes %d\n", size, g_writes);
    return 1;
  }

  memset(output, 0x5a, sizeof(output));
  memset(untouched, 0x5a, sizeof(untouched));
  if (JPEGENC_OpenRAM(&jpeg, output, sizeof(output)) != JPEGE_SUCCESS ||
      !BeginFails(&jpeg)) {
    fprintf(stderr, "RAM setup failed\n");
    return 1;
  }
  size = JPEGENC_Close(&jpeg);
  if (size != 0 || memcmp(output, untouched, sizeof(output)) != 0) {
    fprintf(stderr, "RAM close: size %d, buffer modified\n", size);
    return 1;
  }
  printf("Close after a failed EncodeBegin writes nothing\n");
  return 0;
}
