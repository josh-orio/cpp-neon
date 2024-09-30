#include <algorithm>
// #include <print>
#include <random>
#include <iostream>

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
    // std::print(" {0}", d[i]);
    std::cout << " " << d[i];
  }
  // std::println("");
  std::cout << std::endl;
}
////

// CLASSIC ARRAY MULTIPLICATION (SERIAL)
void amul(float *a, float *b, float *c, uint len) {
  for (int i = 0; i < len; i++) {
    c[i] = a[i] * b[i];
    // std::println("{} {} {}", a[i], b[i], a[i] * b[i]);
  }
}
////

int main() {
  const int len = 128;

  std::array<float, len> a, b, c;

  // load a and b with random values
  std::generate(a.begin(), a.end(), rng);
  std::generate(b.begin(), b.end(), rng);

  // PRINT A
  // std::print("A:");
  std::cout << "A:";
  arr_print(a.data(), 8);

  // PRINT B
  // std::print("B:");
  std::cout << "B:";
  arr_print(b.data(), 8);

  //   namul(a.data(), b.data(), c.data(), len);

  //   for (int i = 0; i < len; i++) {
  //     std::println("{0} * {1} = {2}", a[i], b[i], c[i]);
  //   }

  int test_iterations = 1e5;

  auto start = std::chrono::high_resolution_clock::now(),
       end = std::chrono::high_resolution_clock::now();

  float duration = 0;

  for (int i = 0; i < test_iterations; i++) {
    start = std::chrono::high_resolution_clock::now();
    namul(a.data(), b.data(), c.data(), len);
    end = std::chrono::high_resolution_clock::now();

    duration += std::chrono::duration<double>(end - start).count();
  }

  // std::print("C: ");
  std::cout << "C:";
  arr_print(c.data(), 8);
  // std::println("NEON:    {:.12f} s", duration / test_iterations);
  std::cout << "NEON: " << duration / test_iterations << " s" << std::endl;

  duration = 0;

  for (int i = 0; i < test_iterations; i++) {
    start = std::chrono::high_resolution_clock::now();
    amul(a.data(), b.data(), c.data(), len);
    end = std::chrono::high_resolution_clock::now();

    duration += std::chrono::duration<double>(end - start).count();
  }

  // duration = std::chrono::duration<double>(end - start).count();

  // std::print("C: ");
  std::cout << "C:";
  arr_print(c.data(), 8);
  // std::println("Classic: {:.12f} s", duration / test_iterations);
  std::cout << "Classic: " << duration / test_iterations << " s" << std::endl;

  return 0;
}

// multiple load instructions for one vector multiply

// vector load should just be one instruction?
