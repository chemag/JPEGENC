/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Encode from and to buffers at odd addresses. The 64-bit code store, the
 * grayscale MCU loader, and the YUV422 level shift used typed loads and
 * stores that require natural alignment, which is undefined behavior and
 * faults on strict-alignment targets. Build with -fsanitize=alignment.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jpegenc_test.h"

#define WIDTH 64
#define HEIGHT 64
#define OUTPUT_SIZE (WIDTH * HEIGHT * 8 + 4096)

/* Encode pixels at offset `shift` into an output buffer at the same offset;
 * returns the compressed size, or -1. */
static int Encode(const uint8_t* pixels, int pitch, int pixel_type,
                  int subsample, int shift, uint8_t* copy) {
  static uint8_t input[WIDTH * HEIGHT * 4 + 8];
  static uint8_t output[OUTPUT_SIZE + 8];
  JPEGE_IMAGE jpeg;
  JPEGENCODE encode;
  int size;

  memcpy(input + shift, pixels, (size_t)pitch * HEIGHT);
  if (JPEGOpenRAM(&jpeg, output + shift, OUTPUT_SIZE) != JPEGE_SUCCESS ||
      JPEGEncodeBegin(&jpeg, &encode, WIDTH, HEIGHT, (uint8_t)pixel_type,
                          (uint8_t)subsample, JPEGE_Q_BEST) != JPEGE_SUCCESS ||
      JPEGAddFrame(&jpeg, &encode, input + shift, pitch) != JPEGE_SUCCESS) {
    return -1;
  }
  size = JPEGClose(&jpeg);
  if (size > 0) memcpy(copy, output + shift, (size_t)size);
  return size;
}

/* The misaligned encode must succeed and match the aligned one. */
static int Check(const char* name, int pixel_type, int subsample, int bpp) {
  static uint8_t pixels[WIDTH * HEIGHT * 4];
  static uint8_t aligned[OUTPUT_SIZE], misaligned[OUTPUT_SIZE];
  uint32_t seed = 1;
  int i, size_aligned, size_misaligned;

  for (i = 0; i < WIDTH * HEIGHT * bpp; i++) {
    seed = seed * 1103515245u + 12345u; /* noise: long codes, few 0xff */
    pixels[i] = (uint8_t)(seed >> 16);
  }
  size_aligned = Encode(pixels, WIDTH * bpp, pixel_type, subsample, 0, aligned);
  size_misaligned =
      Encode(pixels, WIDTH * bpp, pixel_type, subsample, 1, misaligned);
  if (size_aligned <= 0 || size_aligned != size_misaligned ||
      memcmp(aligned, misaligned, (size_t)size_aligned) != 0) {
    fprintf(stderr, "%s: sizes %d and %d differ or fail\n", name, size_aligned,
            size_misaligned);
    return 0;
  }
  return 1;
}

int main(void) {
  if (!Check("grayscale", JPEGE_PIXEL_GRAYSCALE, JPEGE_SUBSAMPLE_444, 1) ||
      !Check("rgb888", JPEGE_PIXEL_RGB888, JPEGE_SUBSAMPLE_444, 3) ||
      !Check("yuv422", JPEGE_PIXEL_YUV422, JPEGE_SUBSAMPLE_420, 2)) {
    return 1;
  }
  printf("Unaligned input and output buffers encoded\n");
  return 0;
}
