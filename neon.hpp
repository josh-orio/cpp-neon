#ifndef NEON_HPP
#define NEON_HPP

/* FUNCTIONS USING NEON INSTRUCTIONS & REGISTERS */

// ARM NEON INSTRUCTIONS
#include <arm_neon.h>
////

#include <algorithm>
#include <iostream>

void namul(float *a, float *b, float *c,
           unsigned long len) { /* neon array multiply */
  float32x4_t vec1, vec2, vec3;

  for (int i = 0; i < len; i += 4) {
    // load a and b segments into neon registers
    vec1 = vld1q_f32(&a[i]);
    vec2 = vld1q_f32(&b[i]);

    // parallel multiply
    vec3 = vmulq_f32(vec1, vec2);
    // vec3 = vmulq_f32(vld1q_f32(&a[i]), vld1q_f32(&b[i]));

    // move result back to array (c)
    vst1q_f32(&c[i], vec3);
  }
}

void npamul(float *a, float *b, float *c,
            unsigned long len) { /* neon prefetched array multiply */
  float32x4_t vec1, vec2, vec3;

  int l1s = 196608; // Apple M3 Pro l1 cache per core (bytes)

  int pfs = l1s / 4; // prefetch size (a bit arbitrary but sections of all 3
                     // arrays should fit in cache)

  // std::cout << (len * sizeof(float)) / pfs << len << pfs << std::endl;

  for (int i = 0; i < ((len * sizeof(float)) / pfs) + 1; i++) {
    // https://developer.arm.com/documentation/101458/2010/Coding-best-practice/Prefetching-with---builtin-prefetch
    __builtin_prefetch(&a[i + pfs], 0,
                       0); // prepare the prefetch for a read, the data can be
                           // removed from the cache after the access
    __builtin_prefetch(&b[i + pfs], 0, 0);
    __builtin_prefetch(&c[i + pfs], 1,
        0); // prepare the prefetch for a write to the memory, the data can be
            // removed from the cache after the access

    // for (int ii = i; ii < std::min((unsigned long)i + pfs, len); ii++) {
    int bound = std::min((unsigned long) i+pfs, len);
    for (int ii = i; ii < bound; ii += 4) {
      // load a and b segments into neon registers
      vec1 = vld1q_f32(&a[ii]);
      vec2 = vld1q_f32(&b[ii]);

      // parallel multiply
      vec3 = vmulq_f32(vec1, vec2);

      // move result back to array (c)
      vst1q_f32(&c[ii], vec3);
    }
  }
}

void nwamul(float *a, float *b, float *c,
            unsigned long len) { /* neon widened array multiply */
  float32x4x4_t bankA, bankB, bankC;

  for (int ii = 0; ii < len; ii += 16) {
    // load a and b segments into neon registers
    bankA = vld1q_f32_x4(&a[ii]);
    bankB = vld1q_f32_x4(&b[ii]);

    // parallel multiply
    bankC.val[0] = vmulq_f32(bankA.val[0], bankB.val[0]);
    bankC.val[1] = vmulq_f32(bankA.val[1], bankB.val[1]);
    bankC.val[2] = vmulq_f32(bankA.val[2], bankB.val[2]);
    bankC.val[3] = vmulq_f32(bankA.val[3], bankB.val[3]);

    // move result back to array (c)
    vst1q_f32_x4(&c[ii], bankC);
  }
}

#endif /* NEON_HPP */