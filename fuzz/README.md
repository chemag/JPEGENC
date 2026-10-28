---
type: "developer-guide"
title: "Opt-in JPEGENC fuzzing"
description: "Build and run the standalone JPEGENC libFuzzer target explicitly."
tags: [jpegenc, fuzzing, libfuzzer, sanitizers]
author: "Chema Gonzalez"
generated_by: "Metamate"
date: "2026-10-27"
---

# 1. Scope

This directory is a standalone host-only CMake project, modeled on the
JPEGDEC fuzzing setup. It is not referenced by the normal Arduino or Linux
builds. `BUILD_CLANG_FUZZER` defaults to `OFF`, even when configuring this
project directly. Nothing builds or runs fuzzers unless explicitly enabled.

`jpegenc_encode_fuzzer` drives the C API. The first six input bytes choose
the image size (1 to 256 pixels per side), pixel type, subsampling, quality,
input misalignment, row padding, `JPEGENC_AddFrame()` or per-MCU
`JPEGENC_AddMCU()` calls, and RAM or callback output, including small RAM
buffers that should end in `JPEGE_NO_BUFFER`. Planar YUV420 and NV12 use
`JPEGENC_AddFrameYUV420P()` and `JPEGENC_AddFrameNV12()`. The remaining bytes
fill the pixels. Every input plane is allocated to its exact size, so reads
past the last row or column are caught. The header comment of
`jpegenc_encode_fuzzer.cc` lists the byte layout.

The encoder and harness are instrumented with libFuzzer coverage,
AddressSanitizer, and UndefinedBehaviorSanitizer, and every UBSan error is
fatal so libFuzzer preserves reproducers. The C++ wrapper is not covered.

# 2. Explicit build

Run fuzzing on the devserver, not Cardoso. Keep build products outside the
source checkout. From the JPEGENC repository, choose an external build path,
for example `build="$HOME/workspace/jpegenc-fuzz"`, then run:

* `cmake -S fuzz -B "$build" -G Ninja -DBUILD_CLANG_FUZZER=ON`
  with `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`
  and `-DCMAKE_BUILD_TYPE=RelWithDebInfo` on the same command.
* `cmake --build "$build" --parallel 2`.

Clang and its libFuzzer/sanitizer runtimes are required. GCC is rejected only
when fuzzing is enabled. No packages, corpora, or submodules are fetched by
CMake. Sanitizer flags are target-local and do not change normal builds.

# 3. Seeds and operation

Generate seeds only when requested:

* `python3 fuzz/seed_corpus.py --output-dir "$build/seeds"`.

The script writes one smooth test pattern per pixel type and subsampling, at
sizes that are and are not whole MCUs. Then:

* `mkdir -p "$build/corpus" "$build/artifacts"`, and copy the seeds into
  `"$build/corpus/"`.
* Run `"$build/jpegenc_encode_fuzzer" "$build/corpus"`
  with `-runs=1000 -timeout=10 -rss_limit_mb=512`
  and `-artifact_prefix="$build/artifacts/"` on the same command.

Use bounded runs for smoke testing; start long campaigns explicitly. Replay
a finding with the executable and the artifact path, for example
`"$build/jpegenc_encode_fuzzer" "$build/artifacts/crash-..."`. Minimize and
fix encoder findings as separate patches with targeted tests; adding this
harness does not establish that the encoder is fuzz-clean.

# 4. Build-gate regression test

Run `python3 fuzz/test_build_options.py --work-dir "$build/gate-tests"`.
It checks that default and explicit-OFF builds contain no fuzz targets,
encoder objects, or sanitizer flags, and that an explicit-ON GCC build fails
with the Clang requirement. It does not execute a fuzzing campaign.
