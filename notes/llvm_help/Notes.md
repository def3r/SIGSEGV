# Notes
- Intrinsic functions are mappings to low level instructions provided as high level C/C++ functions.
- Intrinsics are hardware specific.

- SIMD Instruction Set:
  1. x86 (Intel/AMD) :
    - `MMX` : 64-bit SIMD registers and allows to only operate on integers
    - `SSE` : 128-bit SIMD registers and allows flops
    - `AVX` : 256-bir SIMD registers
  2. ARM :
    - `NEON` : (mandatory for android devices! lol)

## Theory
- `FP64` Double Precision FP format:
```
[ 1 ][     11     ][           52            ]  =  64 bits
Sign    Exponent           Precision (fraction)
```

## Flags
### GNU
- AVX `-mavx` or `-mavx2`
