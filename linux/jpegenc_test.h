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

#ifdef __cplusplus
// JPEGENC.h declares its C API for C compilers only; the fuzz
// harnesses are C++.
extern "C" {
int JPEGOpenRAM(JPEGE_IMAGE *pJPEG, uint8_t *pData, int iDataSize);
int JPEGOpenFile(JPEGE_IMAGE *pJPEG, const char *szFilename);
int JPEGEncodeBegin(JPEGE_IMAGE *pJPEG, JPEGENCODE *pEncode, int iWidth, int iHeight, uint8_t ucPixelType, uint8_t ucSubSample, uint8_t ucQFactor);
int JPEGEncodeEnd(JPEGE_IMAGE *pJPEG);
int JPEGAddMCU(JPEGE_IMAGE *pJPEG, JPEGENCODE *pEncode, uint8_t *pPixels, int iPitch);
int JPEGAddFrameYUV420P(JPEGE_IMAGE *pJPEG, JPEGENCODE *pEncode, const uint8_t *pY, int iYPitch, const uint8_t *pCb, int iCbPitch, const uint8_t *pCr, int iCrPitch);
int JPEGAddFrameNV12(JPEGE_IMAGE *pJPEG, JPEGENCODE *pEncode, const uint8_t *pY, int iYPitch, const uint8_t *pUV, int iUVPitch);
int JPEGGetLastError(JPEGE_IMAGE *pJPEG);
#endif

int JPEGOpen(JPEGE_IMAGE *pJPEG, const char *szFilename, JPEGE_OPEN_CALLBACK *pfnOpen,
             JPEGE_CLOSE_CALLBACK *pfnClose, JPEGE_READ_CALLBACK *pfnRead,
             JPEGE_WRITE_CALLBACK *pfnWrite, JPEGE_SEEK_CALLBACK *pfnSeek);
int JPEGClose(JPEGE_IMAGE *pJPEG);
int JPEGAddFrame(JPEGE_IMAGE *pJPEG, JPEGENCODE *pEncode, uint8_t *pPixels, int iPitch);

#ifdef __cplusplus
}
#endif

#endif // JPEGENC_TEST_H
