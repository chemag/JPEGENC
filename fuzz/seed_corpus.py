#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Write seed inputs for jpegenc_encode_fuzzer.

Each seed is the six-byte parameter header the harness documents followed by
a smooth test pattern, one per pixel type and subsampling, at sizes that are
and are not whole MCUs.
"""

import argparse
import math
from pathlib import Path

PIXEL_TYPES = 8  # JPEGE_PIXEL_COUNT
SIZES = [(16, 16), (17, 9), (40, 33)]


def pattern(length: int) -> bytes:
    return bytes(int(128 + 100 * math.sin(i / 7.0)) & 0xFF for i in range(length))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    count = 0
    for pixel_type in range(PIXEL_TYPES):
        for subsample in (0, 1):
            for width, height in SIZES:
                header = bytes([width - 1, height - 1, pixel_type,
                                subsample | (1 << 1), 0, 0])
                data = header + pattern(width * height * 4)
                name = f"seed-t{pixel_type}-s{subsample}-{width}x{height}"
                (args.output_dir / name).write_bytes(data)
                count += 1
    print(f"wrote {count} seeds to {args.output_dir}")


if __name__ == "__main__":
    main()
