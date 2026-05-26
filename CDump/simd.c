// Consider only SSE2
// $ gcc -march=native -msse2
//
// ref: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE2&ig_expand=89,2927,7021,2927

#include <emmintrin.h>

// SSE2 only contains a 16bit int extract
#define _mm_extract_epi32_SSE2(v, vec, i)    \
  int v##00 = _mm_extract_epi16(vec, i);     \
  int v##01 = _mm_extract_epi16(vec, i + 1); \
  int v = (v##01 << 16) + v##00;

int main() {
  // [ 128 bits ] -> [ 32 bits x 4 lanes]
  //              -> [ 16 bits x 8 lanes]
  __m128i vec1 = _mm_set_epi32(4, 3, 2, 1);
  __m128i vec2 = _mm_set_epi32(10, 9, 8, 7);
  __m128i res = _mm_add_epi32(vec1, vec2);

  _mm_extract_epi32_SSE2(val0, res, 0);
  _mm_extract_epi32_SSE2(val1, res, 2);
  _mm_extract_epi32_SSE2(val2, res, 4);
  _mm_extract_epi32_SSE2(val3, res, 6);

  int val = val0 + val1 + val2 + val3;

  return val;
}

// - No opt is mind blowingly stupid
// - -O2 is absolutely chad
