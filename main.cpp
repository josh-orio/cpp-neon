#include <algorithm>
#include <format>
#include <iostream>
#include <random>

#include "neon.hpp"

// RANDOM NUMBER GENERATOR
std::random_device rd;
std::default_random_engine re = std::default_random_engine(rd());
std::uniform_real_distribution<double> urd =
    std::uniform_real_distribution<double>(0, 1);
float rng() { return (int)(urd(re) * 100) % 100; }
////

// PRINT ARRAY
template <typename T> void arr_print(T *d, int len) {
  for (int i = 0; i < len; i++) {
    std::cout << " " << d[i];
  }
  std::cout << std::endl;
}
////

// CLASSIC ARRAY MULTIPLICATION (SERIAL)
void amul(float *a, float *b, float *c, uint len) {
  for (int i = 0; i < len; i++) {
    c[i] = a[i] * b[i];
  }
}
////

int main() {
  const int len = 128;

  alignas(256) std::array<float, len> a, b, c;

  // timing counters
  double d_amul = 0, d_namul = 0, d_npamul = 0, d_nwamul = 0;

  long test_iterations = 1e6;

  auto start = std::chrono::high_resolution_clock::now(),
       end = std::chrono::high_resolution_clock::now();

  // std::generate(a.begin(), a.end(), rng);
  // std::generate(b.begin(), b.end(), rng);
  //    start = std::chrono::high_resolution_clock::now();
  // npamul(a.data(), b.data(), c.data(), len);
  // end = std::chrono::high_resolution_clock::now();
  // d_nwamul += std::chrono::duration<double>(end - start).count();
  // arr_print(c.data(), 32);

  // return 0;

  for (int i = 0; i < test_iterations; i++) {
    // load a and b with random values
    std::generate(a.begin(), a.end(), rng);
    std::generate(b.begin(), b.end(), rng);

    start = std::chrono::high_resolution_clock::now();
    amul(a.data(), b.data(), c.data(), len);
    end = std::chrono::high_resolution_clock::now();
    d_amul += std::chrono::duration<double>(end - start).count();
    if (i == 0) {
      arr_print(c.data(), 32);
    }
    std::fill(c.begin(), c.end(), 0);

    start = std::chrono::high_resolution_clock::now();
    namul(a.data(), b.data(), c.data(), len);
    end = std::chrono::high_resolution_clock::now();
    d_namul += std::chrono::duration<double>(end - start).count();
    if (i == 0) {
      arr_print(c.data(), 32);
    }
    std::fill(c.begin(), c.end(), 0);

    start = std::chrono::high_resolution_clock::now();
    npamul(a.data(), b.data(), c.data(), len);
    end = std::chrono::high_resolution_clock::now();
    d_npamul += std::chrono::duration<double>(end - start).count();
    if (i == 0) {
      arr_print(c.data(), 32);
    }
    std::fill(c.begin(), c.end(), 0);

    start = std::chrono::high_resolution_clock::now();
    nwamul(a.data(), b.data(), c.data(), len);
    end = std::chrono::high_resolution_clock::now();
    d_nwamul += std::chrono::duration<double>(end - start).count();
    if (i == 0) {
      arr_print(c.data(), 32);
    }
    std::fill(c.begin(), c.end(), 0);
  }

  // // PRINT A
  // // std::print("A:");
  // std::cout << "A:";
  // arr_print(a.data(), 8);

  // // PRINT B
  // // std::print("B:");
  // std::cout << "B:";
  // arr_print(b.data(), 8);

  std::cout << std::format("{:<20}| {:.12f} s | {:.4f} GFLOPS", "Classic",
                           d_amul / test_iterations,
                           128 * (1 / (d_amul / test_iterations)) / 1e9)
            << std::endl;

  std::cout << std::format("{:<20}| {:.12f} s | {:.4f} GFLOPS", "NEON",
                           d_namul / test_iterations,
                           128 * (1 / (d_namul / test_iterations)) / 1e9)
            << std::endl;

  std::cout << std::format("{:<20}| {:.12f} s | {:.4f} GFLOPS", "NEON Prefetched",
                           d_npamul / test_iterations,
                           128 * (1 / (d_npamul / test_iterations)) / 1e9)
            << std::endl;

  std::cout << std::format("{:<20}| {:.12f} s | {:.4f} GFLOPS", "NEON Multi-Fetch",
                           d_nwamul / test_iterations,
                           128 * (1 / (d_nwamul / test_iterations)) / 1e9)
            << std::endl;

  return 0;
}
