#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Regression test: fuzz targets and sanitizer flags require explicit opt-in."""

import argparse
from pathlib import Path
import subprocess
import tempfile


def configure(source: Path, build: Path, options: list[str]) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["cmake", "-S", str(source), "-B", str(build), "-G", "Ninja",
         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", *options],
        capture_output=True, text=True, check=False,
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work-dir", type=Path, required=True)
    args = parser.parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=True)
    source = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory(prefix="fuzz-build-options-", dir=args.work_dir) as temp:
        root = Path(temp)
        for label, options in [("default", []), ("off", ["-DBUILD_CLANG_FUZZER=OFF"])]:
            build = root / label
            result = configure(source, build, options)
            assert result.returncode == 0, result.stdout + result.stderr
            ninja = (build / "build.ninja").read_text()
            assert "jpegenc_encode_fuzzer" not in ninja, ninja
            assert "-fsanitize" not in ninja, ninja
            assert "jpegenc.c.o" not in ninja, ninja
            subprocess.run(["cmake", "--build", str(build)], check=True)
            print(f"{label}: no fuzz targets or sanitizer flags")
        result = configure(source, root / "gcc-on", [
            "-DBUILD_CLANG_FUZZER=ON", "-DCMAKE_C_COMPILER=gcc",
            "-DCMAKE_CXX_COMPILER=g++",
        ])
        assert result.returncode != 0, "GCC unexpectedly accepted libFuzzer build"
        assert "requires Clang" in result.stdout + result.stderr
        print("gcc-on: rejected with explicit Clang requirement")


if __name__ == "__main__":
    main()
