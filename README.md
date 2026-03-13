# Huffman Compression Engine 🗜️

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-CMake-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

A high-performance, strictly memory-safe C++ command-line utility for lossless file compression and decompression using canonical Huffman Coding.

## ✨ Engineering Highlights

* **Arena-Allocated Tree:** The prefix tree is stored as a flat `std::vector<Node>` with integer indices. This guarantees contiguous memory allocation, eliminating heap scatter and drastically reducing CPU cache misses.
* **O(1) Frequency Maps:** Replaced standard hash maps with fixed `std::array<uint64_t, 256>` for direct index lookups, achieving zero-collision and zero-allocation frequency counting.
* **Bitwise Integer Encoding:** Variable-length codes are stored natively as `uint64_t`. Entire codewords are emitted to the buffer via a single bitwise shift, bypassing slow string-character iterations.
* **64KB Buffered I/O:** Custom `BitWriter` and `BitReader` classes batch data into 64KB blocks, minimizing costly system calls during file stream operations.
* **Deterministic Integrity:** Node sorting enforces a strict character-value tie-breaker, guaranteeing identical tree structures and seamless cross-platform decompression.

## ⚙️ Pipeline Architecture

```mermaid
flowchart TD
    classDef file fill:#2d3436,stroke:#636e72,color:#dfe6e9
    classDef buffer fill:#0984e3,stroke:#74b9ff,color:#fff
    classDef struct fill:#fdcb6e,stroke:#ffeaa7,color:#2d3436
    classDef tree fill:#00b894,stroke:#55efc4,color:#fff

    subgraph IOStream ["File I/O Pipeline"]
        direction TB
        A[Input File]:::file --> B(64KB Read Buffer):::buffer
        G(64KB Write Buffer):::buffer --> H[Compressed .huf Data]:::file
    end

    subgraph CoderCore ["Compression Engine"]
        direction TB
        B -.->|Count| C{O(1) Freq Array}:::struct
        C -->|Sort| D[Min-Heap]:::struct
        D -->|Build| E((Arena Tree)):::tree
        E -->|Map| F[Bit Dictionary]:::struct
        B -.->|Encode| F
        F -->|Shift Bits| G
    end

    style IOStream fill:#2f3542,stroke:#57606f,color:#fff
    style CoderCore fill:#1e272e,stroke:#485460,color:#fff
🚀 Getting Started
Prerequisites
A modern C++17 compiler (GCC, Clang, or MSVC)

CMake (v3.10+)

Build Instructions
Bash
git clone [https://github.com/Komal-ai417/HuffmanProject.git](https://github.com/Komal-ai417/HuffmanProject.git)
cd HuffmanProject

mkdir build && cd build
cmake ..
cmake --build . --config Release
💻 Usage
The executable provides a streamlined, parameter-driven interface.

Compress a file:

Bash
./huffman -c <input_file> <output_file.huf>
Decompress a file:

Bash
./huffman -d <input_file.huf> <restored_file.txt>

---

This version gets straight to the technical merit of your C++ and algorithmic skills. 

Would you like me to write a quick `huffman_ci.yml` file so you can add a GitHub Action that automatically compiles and tests your code every time you push a commit?
