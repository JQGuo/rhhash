## Overview

These are implementations of Chained Hashing, Linear Probing, and Robin Hood hash tables.

I wanted to learn about Robin Hood hashing and its properties, and provided a very simple benchmark to compare it with linear probing using the probe length as the metric.

The benchmark adds random keys until the table is almost full (in terms of load factor), then performs deletions for half of those keys. Since generating keys for insertion and deletion are random, there may be much fewer deletions than insertions. In any case, the goal was to see how the probe length changes when using a combination of operations.

You should see that the mean stays around the same value for all 3 hash tables, but the variance for Robin Hood hashing is much lower. The worst case runtime should be reduced, and runtimes between operations should be more consistent.

The code was written with readability in mind, and optimization was not the goal, so Robin Hood Hashing may take longer than Linear Probing if timed.

To build, run `g++ bench.cpp -std=c++11`

