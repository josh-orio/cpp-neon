#ifndef NEON_HPP
#define NEON_HPP

/* FUNCTIONS USING NEON INSTRUCTIONS & REGISTERS */

// ARM NEON INSTRUCTIONS
#include <arm_neon.h>
////

void namul(float *a, float *b, float *c, unsigned long len) { /* neon array multiply */
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

#endif /* NEON_HPP */