/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * C entry points the unit tests use: JPEGENC.h declares JPEGOpenRAM() and
 * JPEGGetLastError() for C but nothing defines them, and it does not declare
 * JPEGAddFrame(). jpegenc_test_api.c defines the missing functions.
 */
#ifndef JPEGENC_TEST_H
#define JPEGENC_TEST_H

#include "../src/JPEGENC.h"

int JPEGOpen(JPEGE_IMAGE *pJPEG, const char *szFilename, JPEGE_OPEN_CALLBACK *pfnOpen,
             JPEGE_CLOSE_CALLBACK *pfnClose, JPEGE_READ_CALLBACK *pfnRead,
             JPEGE_WRITE_CALLBACK *pfnWrite, JPEGE_SEEK_CALLBACK *pfnSeek);
int JPEGClose(JPEGE_IMAGE *pJPEG);
int JPEGAddFrame(JPEGE_IMAGE *pJPEG, JPEGENCODE *pEncode, uint8_t *pPixels, int iPitch);

#endif // JPEGENC_TEST_H
