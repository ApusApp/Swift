Swift
=====
Swift is a basic C++ library on linux platform which using c++11.

Dependencies
=============
- additional platform specific dependencies:

  Ubuntu 13.10 or late 64-bit
    - g++
    - Cmake
    - libgtest-dev
    - libgoogle-glog-dev
    - libgflags-dev
    - unit test need libssl-dev

Build
======
    cd Swift && ./build.sh
    foundational libraries are in Swift/build/debug(release)/lib
    runnable programs of unit test at Swift/build/debug(release)/bin
    strongly recommend running the unit test

Computer Latency Numbers
========================
    L1 cache reference ......................... 0.5 ns
    Branch mispredict ............................ 5 ns
    L2 cache reference ........................... 7 ns
    Mutex lock/unlock ........................... 25 ns
    Main memory reference ...................... 100 ns
    Compress 1K bytes with Zippy ............. 3,000 ns  =   3 µs
    Send 2K bytes over 1 Gbps network ....... 20,000 ns  =  20 µs
    SSD random read ........................ 150,000 ns  = 150 µs
    Read 1 MB sequentially from memory ..... 250,000 ns  = 250 µs
    Round trip within same datacenter ...... 500,000 ns  = 0.5 ms
    Read 1 MB sequentially from SSD* ..... 1,000,000 ns  =   1 ms
    Disk seek ........................... 10,000,000 ns  =  10 ms
    Read 1 MB sequentially from disk .... 20,000,000 ns  =  20 ms
    Send packet CA->Netherlands->CA .... 150,000,000 ns  = 150 ms


