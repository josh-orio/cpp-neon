import numpy as np
import time

a = np.random.rand(128)
b = np.random.rand(128)

duration = 0
for i in range(int(1e6)):
    start = time.time()
    c = np.multiply(a, b)
    # c = a * b
    end = time.time()
    duration += end - start

print(duration / 1e6, "s |", 128 * (1 / (duration / 1e6)) / 1e9, "GFLOPS")
