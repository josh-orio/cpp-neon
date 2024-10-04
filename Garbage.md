This calculation gives us the memory throughput of the program, so clearly we aren't memory limited:
```
Iterations per second (CLASSIC) = (1 / 0.000000016414) = 60923601.8033 I/s
Bytes per iteration = 512 * 3 = 1536 B/I
Gigabytes per iteration = 0.000001536 GB/I
Gigabytes per second = 60923601.8033 * 0.000001536 = ~93.5 GB/s

M3 Pro mem throughput = ~150 GB/s
```

So what's the explanation? Let's look at the assembly for each of these functions...