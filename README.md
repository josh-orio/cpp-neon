# ARM NEON - C++

Using intrinsics for the first time. This project is only a demo to gain some familiarity with ARM's NEON extension (similar to SIMD).

Going forward, I will use NEON to continue hand programming a matrix multiplication in C++ that competes with NumPy, which is built on BLAS.

## Useful Intrinsics Reference

https://arm-software.github.io/acle/neon_intrinsics/advsimd.html

## Performance Analysis

The main loop in main.cpp runs each of the array multiplication functions many times to get an accurate reading of their speed.

```amul()``` does a conventional (C++ only) element-wise multiplication

```namul()``` does an element-wise multiplication but it uses NEON intrinsics to vectorize (vld, vst, vmul)

```npamul()``` does the same as ```namul()``` but attempts to prefetch the array into the cache

```nwamul()``` also does the same as ```namul()``` but loads 16 floats into 4 NEON registers at a time (for each operand array)

Averaged time for a function call over 1e6 iterations:

```
joshorio@mac build % /Users/joshorio/Work/neon/build/neon
Classic             | 0.000000016441 s | 7.7852 GFLOPS
NEON                | 0.000000016474 s | 7.7696 GFLOPS
NEON Prefetched     | 0.000000061122 s | 2.0942 GFLOPS
NEON Multi-Fetch    | 0.000000016496 s | 7.7593 GFLOPS
```

What's interesting is that over 1e6 iterations, none of the NEON implementations is (significantly) faster than the classic scalar implementationn

A likely explanation is that element-wise multiplications are often memory-bound. Each element is only accessed once during the whole function call, which doesn't allow for great use of the processor's cache. Cache prefetching units likely already do a perfect job of fetching array elements that are about to be accessed, but that is just about the best use of cache this type of calculation can manage.

To compare, numpy is even slower which is surprising given how fast numpy.matmul() is.

```
joshorio@mac build % python3 ../np.py
3.067350387573242e-07 s | 0.41729826666873937 GFLOPS
```

## Assembly

Using lldb's disassembly feature, we can look at how these functions are implemented by the compiler, and it shows a lot of interesting details.

### amul()
```
(lldb) dis -a 0x0000000100003d44
neon`amul:
    0x100003d44 <+0>:   cbz    w3, 0x100003df0           ; <+172> at main.cpp:33:1
    0x100003d48 <+4>:   mov    w8, w3
    0x100003d4c <+8>:   cmp    w3, #0x10
    0x100003d50 <+12>:  b.hs   0x100003d5c               ; <+24> at main.cpp
    0x100003d54 <+16>:  mov    x9, #0x0
    0x100003d58 <+20>:  b      0x100003dc4               ; <+128> at main.cpp:29:3
    0x100003d5c <+24>:  mov    x9, #0x0
    0x100003d60 <+28>:  sub    x10, x2, x0
    0x100003d64 <+32>:  cmp    x10, #0x40
    0x100003d68 <+36>:  b.lo   0x100003dc4               ; <+128> at main.cpp:29:3
    0x100003d6c <+40>:  sub    x10, x2, x1
    0x100003d70 <+44>:  cmp    x10, #0x40
    0x100003d74 <+48>:  b.lo   0x100003dc4               ; <+128> at main.cpp:29:3
    0x100003d78 <+52>:  and    x9, x8, #0xfffffff0
    0x100003d7c <+56>:  add    x10, x2, #0x20
    0x100003d80 <+60>:  add    x11, x0, #0x20
    0x100003d84 <+64>:  add    x12, x1, #0x20
    0x100003d88 <+68>:  mov    x13, x9
    0x100003d8c <+72>:  ldp    q0, q1, [x11, #-0x20]
    0x100003d90 <+76>:  ldp    q2, q3, [x11], #0x40
    0x100003d94 <+80>:  ldp    q4, q5, [x12, #-0x20]
    0x100003d98 <+84>:  ldp    q6, q7, [x12], #0x40
    0x100003d9c <+88>:  fmul.4s v0, v0, v4
    0x100003da0 <+92>:  fmul.4s v1, v1, v5
    0x100003da4 <+96>:  fmul.4s v2, v2, v6
    0x100003da8 <+100>: fmul.4s v3, v3, v7
    0x100003dac <+104>: stp    q0, q1, [x10, #-0x20]
    0x100003db0 <+108>: stp    q2, q3, [x10], #0x40
    0x100003db4 <+112>: subs   x13, x13, #0x10
    0x100003db8 <+116>: b.ne   0x100003d8c               ; <+72> at main.cpp:30:12
    0x100003dbc <+120>: cmp    x9, x8
    0x100003dc0 <+124>: b.eq   0x100003df0               ; <+172> at main.cpp:33:1
    0x100003dc4 <+128>: lsl    x12, x9, #2
    0x100003dc8 <+132>: add    x10, x2, x12
    0x100003dcc <+136>: add    x11, x1, x12
    0x100003dd0 <+140>: add    x12, x0, x12
    0x100003dd4 <+144>: sub    x8, x8, x9
    0x100003dd8 <+148>: ldr    s0, [x12], #0x4
    0x100003ddc <+152>: ldr    s1, [x11], #0x4
    0x100003de0 <+156>: fmul   s0, s0, s1
    0x100003de4 <+160>: str    s0, [x10], #0x4
    0x100003de8 <+164>: subs   x8, x8, #0x1
    0x100003dec <+168>: b.ne   0x100003dd8               ; <+148> at main.cpp:30:12
    0x100003df0 <+172>: ret
```

This function is long, maybe thats why its slow. It also has more branches, adds and subs than would make sense to me, I would think there should only be one branch at the end of the loop. The block of 4 loads, 4 multiplies and 2 stores makes sense, it seems that the compiler has partially vectorised the loop.

### namul()
```
(lldb) dis -a 0x0000000100003b54
neon`namul:
    0x100003b54 <+0>:  cbz    x3, 0x100003b78           ; <+36> at neon.hpp:29:1
    0x100003b58 <+4>:  mov    x8, #0x0
    0x100003b5c <+8>:  ldr    q0, [x0], #0x10
    0x100003b60 <+12>: ldr    q1, [x1], #0x10
    0x100003b64 <+16>: fmul.4s v0, v0, v1
    0x100003b68 <+20>: str    q0, [x2], #0x10
    0x100003b6c <+24>: add    x8, x8, #0x4
    0x100003b70 <+28>: cmp    x8, x3
    0x100003b74 <+32>: b.lo   0x100003b5c               ; <+8> at neon.hpp:19:12
    0x100003b78 <+36>: ret
```

Clearly this function is much simpler. A and B are loaded into a register, get multiplied and immediately stored. I guess this loop is much quicker to run but has to run more often to complete the array multiplication than the ```amul()``` binary.

### npamul()
```
(lldb) dis -a 0x0000000100003ba4
neon`npamul:
    0x100003b7c <+0>:   mov    x9, #0x0
    0x100003b80 <+4>:   lsl    x8, x3, #2
    0x100003b84 <+8>:   mov    x10, #-0x5555555555555556
    0x100003b88 <+12>:  movk   x10, #0xaaab
    0x100003b8c <+16>:  umulh  x8, x8, x10
    0x100003b90 <+20>:  lsr    x8, x8, #15
    0x100003b94 <+24>:  mov    x10, x0
    0x100003b98 <+28>:  mov    x11, x1
    0x100003b9c <+32>:  mov    x12, x2
    0x100003ba0 <+36>:  b      0x100003bc0               ; <+68> at neon.hpp:44:29
    0x100003ba4 <+40>:  add    x13, x9, #0x1
    0x100003ba8 <+44>:  add    x12, x12, #0x4
    0x100003bac <+48>:  add    x11, x11, #0x4
    0x100003bb0 <+52>:  add    x10, x10, #0x4
    0x100003bb4 <+56>:  cmp    x9, x8
    0x100003bb8 <+60>:  mov    x9, x13
    0x100003bbc <+64>:  b.eq   0x100003c2c               ; <+176> at neon.hpp:66:1
    0x100003bc0 <+68>:  add    x13, x9, #0xc, lsl #12    ; =0xc000
    0x100003bc4 <+72>:  lsl    x14, x9, #2
    0x100003bc8 <+76>:  add    x15, x0, x14
    0x100003bcc <+80>:  add    x15, x15, #0x30, lsl #12  ; =0x30000
    0x100003bd0 <+84>:  prfm   pldl1strm, [x15]
    0x100003bd4 <+88>:  add    x15, x1, x14
    0x100003bd8 <+92>:  add    x15, x15, #0x30, lsl #12  ; =0x30000
    0x100003bdc <+96>:  prfm   pldl1strm, [x15]
    0x100003be0 <+100>: add    x14, x2, x14
    0x100003be4 <+104>: add    x14, x14, #0x30, lsl #12  ; =0x30000
    0x100003be8 <+108>: prfm   pstl1strm, [x14]
    0x100003bec <+112>: cmp    x13, x3
    0x100003bf0 <+116>: csel   x13, x13, x3, lo
    0x100003bf4 <+120>: cmp    x9, w13, sxtw
    0x100003bf8 <+124>: mov    x14, x9
    0x100003bfc <+128>: mov    x15, x10
    0x100003c00 <+132>: mov    x16, x11
    0x100003c04 <+136>: mov    x17, x12
    0x100003c08 <+140>: b.ge   0x100003ba4               ; <+40> at neon.hpp:42:59
    0x100003c0c <+144>: ldr    q0, [x15], #0x10
    0x100003c10 <+148>: ldr    q1, [x16], #0x10
    0x100003c14 <+152>: fmul.4s v0, v0, v1
    0x100003c18 <+156>: str    q0, [x17], #0x10
    0x100003c1c <+160>: add    w14, w14, #0x4
    0x100003c20 <+164>: cmp    w14, w13
    0x100003c24 <+168>: b.lt   0x100003c0c               ; <+144> at neon.hpp:56:14
    0x100003c28 <+172>: b      0x100003ba4               ; <+40> at neon.hpp:42:59
    0x100003c2c <+176>: ret
```

Long and slow... boring! Also only one multiply instruction per iteration. Prefetching was unlikely to ever help, the linear access pattern of the function makes it pretty easy for a prefetching unit to guess which parts of memory are going to be accessed next, as such, the array data is likely already in the core cache and the prefetch call just gets in the way.

### nwamul()
```
(lldb) dis -a 0x0000000100003c30
neon`nwamul:
    0x100003c30 <+0>:  cbz    x3, 0x100003c60           ; <+48> at neon.hpp:86:1
    0x100003c34 <+4>:  mov    x8, #0x0
    0x100003c38 <+8>:  ld1.4s { v0, v1, v2, v3 }, [x0], #64
    0x100003c3c <+12>: ld1.4s { v4, v5, v6, v7 }, [x1], #64
    0x100003c40 <+16>: fmul.4s v16, v0, v4
    0x100003c44 <+20>: fmul.4s v17, v1, v5
    0x100003c48 <+24>: fmul.4s v18, v2, v6
    0x100003c4c <+28>: fmul.4s v19, v3, v7
    0x100003c50 <+32>: add    x8, x8, #0x10
    0x100003c54 <+36>: st1.4s { v16, v17, v18, v19 }, [x2], #64
    0x100003c58 <+40>: cmp    x8, x3
    0x100003c5c <+44>: b.lo   0x100003c38               ; <+8> at neon.hpp:74:13
    0x100003c60 <+48>: ret
```

Another short loop, this time with multiple load, multiply and store instructions. The factor holding this function back is most likely the fact that these instructions take more clock cycles to complete.
