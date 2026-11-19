# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.16)
project(jpegenc_fuzz LANGUAGES C CXX)

option(BUILD_CLANG_FUZZER "Build JPEGENC libFuzzer targets" OFF)

if(NOT BUILD_CLANG_FUZZER)
  message(STATUS "JPEGENC fuzzing disabled; set BUILD_CLANG_FUZZER=ON to enable")
  return()
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang" OR
   NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "BUILD_CLANG_FUZZER requires Clang for C and C++")
endif()

# Instrument the encoder itself, not only the libFuzzer entry point.
# Flags are target-local and cannot leak into normal JPEGENC builds.
set(JPEGENC_SANITIZERS -fsanitize=address,undefined
    -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
add_library(jpegenc_fuzz_core STATIC ../linux/jpegenc_test_core.c ../linux/jpegenc_test_api.c)
target_include_directories(jpegenc_fuzz_core PUBLIC ../src ../linux)
target_compile_definitions(jpegenc_fuzz_core PUBLIC JPEGENC_NO_ARDUINO)
target_compile_options(jpegenc_fuzz_core PRIVATE
    ${JPEGENC_SANITIZERS} -fsanitize=fuzzer-no-link)

add_executable(jpegenc_encode_fuzzer jpegenc_encode_fuzzer.cc)
target_compile_features(jpegenc_encode_fuzzer PRIVATE cxx_std_11)
target_compile_options(jpegenc_encode_fuzzer PRIVATE
    ${JPEGENC_SANITIZERS} -fsanitize=fuzzer-no-link -Wall -Wextra -Werror)
target_link_options(jpegenc_encode_fuzzer PRIVATE
    ${JPEGENC_SANITIZERS} -fsanitize=fuzzer)
target_link_libraries(jpegenc_encode_fuzzer PRIVATE jpegenc_fuzz_core)
