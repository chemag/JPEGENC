/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Packed YUV422 input supports only 4:2:0 output. With 4:4:4 the MCU loaders
 * read each YUYV pair as a 32-bit RGB pixel, so they produced wrong colors
 * and read past the end of the last row. EncodeBegin() must reject that
 * combination, and 4:2:0 must still encode an odd-size image from an exactly
 * sized buffer. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "jpegenc_test.h"

#define WIDTH 17
#define HEIGHT 9
#define PITCH (((WIDTH + 1) & ~1) * 2) /* whole YUYV pairs */

static int Encode(int subsample, const uint8_t* pixels, int* size) {
  static uint8_t output[8192];
  JPEGE_IMAGE jpeg;
  JPEGENCODE encode;
  int rc;

  if (JPEGOpenRAM(&jpeg, output, sizeof(output)) != JPEGE_SUCCESS) {
    return -1;
  }
  rc = JPEGEncodeBegin(&jpeg, &encode, WIDTH, HEIGHT, JPEGE_PIXEL_YUV422,
                           (uint8_t)subsample, JPEGE_Q_HIGH);
  if (rc == JPEGE_SUCCESS) {
    rc = JPEGAddFrame(&jpeg, &encode, (uint8_t*)pixels, PITCH);
  }
  *size = JPEGClose(&jpeg);
  return rc;
}

int main(void) {
  uint8_t* pixels = malloc(PITCH * HEIGHT);
  int i, size, rc444, rc420;

  if (pixels == NULL) return 1;
  for (i = 0; i < PITCH * HEIGHT; i++) pixels[i] = (uint8_t)(i * 7);
  rc444 = Encode(JPEGE_SUBSAMPLE_444, pixels, &size);
  rc420 = Encode(JPEGE_SUBSAMPLE_420, pixels, &size);
  free(pixels);
  if (rc444 != JPEGE_INVALID_PARAMETER || rc420 != JPEGE_SUCCESS || size < 4) {
    fprintf(stderr, "4:4:4 rc %d, 4:2:0 rc %d size %d\n", rc444, rc420, size);
    return 1;
  }
  printf("YUV422 4:4:4 rejected, 4:2:0 encoded: %d bytes\n", size);
  return 0;
}
