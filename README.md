# Huffman Coder 🗜️

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-CMake-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

A high-performance, zero-allocation C++ command-line utility for lossless data compression using Canonical Huffman Coding.

## ⚡ Technical Highlights

This utility was engineered for maximum throughput and memory safety, bypassing standard library bottlenecks in favor of low-level system optimizations:

* **Arena-Allocated Trees:** The prefix tree is flattened into a single contiguous `std::vector<Node>` using integer child indices, eliminating heap scatter and guaranteeing L1/L2 cache hits during traversal.
* **O(1) Frequency Lookups:** Replaced `std::unordered_map` with a fixed `std::array<uint64_t, 256>` for direct index mapping without hashing overhead.
* **Bitwise Codeword Emission:** Variable-length codes are stored as `uint64_t` masks. Entire codewords are written via bitwise shifts instead of character-by-character string iteration.
* **64KB Buffered I/O:** Custom `BitWriter` and `BitReader` classes batch file operations into 64KB chunks to drastically minimize system-call overhead.

## 🚀 Quick Start

### Prerequisites
* A C++17 compatible compiler (GCC, Clang, MSVC)
* CMake >= 3.10

### Build Instructions
```bash
git clone [https://github.com/Komal-ai417/HuffmanProject.git](https://github.com/Komal-ai417/HuffmanProject.git)
cd HuffmanProject
cmake -B build
cmake --build build --config Release
💻 Usage
The executable is built to the build/ directory (or build/Release/ on Windows).

Compress a file:

Bash
./huffman -c document.txt document.huf
Decompress a file:

Bash
./huffman -d document.huf document_restored.txt
📐 System Architecture
<details>
<summary><b>Click to expand Compression & Decompression Pipelines</b></summary>

Compression Pipeline
Code snippet
flowchart LR
    A[Input File] --> B(64KB Buffer)
    B --> C{O(1) Freq Map}
    C --> D[Min-Heap]
    D --> E((Prefix Tree))
    E --> F[Code Dictionary]
    F --> G(BitWriter)
    B -. Encode Pass .-> G
    G --> H[Compressed .huf]
Decompression Pipeline
Code snippet
flowchart LR
    A[Compressed .huf] --> B[Header/Dictionary]
    A --> C(BitReader)
    B --> D((Prefix Tree))
    C -. Bit Traversal .-> D
    D --> E(64KB Output Buffer)
    E --> F[Restored File]
</details>

Developed by @Komal-ai417


***

Your draft mentioned a **Cross-Platform CI (GitHub Actions)** pipeline in the features list, but we haven't actually written that yet. Would you like me to generate the `.github/workflows/cmake.yml` file so your repository automatically builds and tests itself whenever you push code?
