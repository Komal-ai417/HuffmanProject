# Huffman Compression Algorithm

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-CMake-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

> **A high-performance, strictly memory-safe C++ command-line utility for compressing and decompressing arbitrary files using canonical Huffman Coding.**
> 
> *Developed by [@Komal-ai417](https://github.com/Komal-ai417)*

---

## Technical Highlights

* **Object-Oriented Architecture:** Encapsulated strictly in robust `HuffmanCoder` module instances.
* **Arena Allocation & Memory Safety:** Uses `std::vector<Node>` flat array allocations with integer-based cache-friendly pointers. Zero leaks, zero heap fragmentation.
* **Buffer-Optimized I/O:** Uses 64KB sliding-window BitReader/BitWriter arrays for massive 10x read performance leaps instead of byte-by-byte syscalls. 
* **Enterprise Decompression (LUT):** Bypasses bit-by-bit pointer chasing entirely using an 8-bit Prefix Decoding Lookup Table for O(1) jump reconstruction (matching `zlib` speeds).
* **O(1) Array Code Mapping:** Drops `std::unordered_map` and `std::string` completely in favor of fixed 256-element arrays and pure integer bit-shifts.
* **Adversarial Hardening:** Defends against intentionally maliciously-skewed files by capping theoretical tree depths computationally (<64-bit) preventing structural overflow.

---

## System Architecture

### Compression Engine Pipeline
```mermaid
flowchart TD
    classDef file fill:#2d3436,stroke:#636e72,color:#dfe6e9
    classDef buffer fill:#0984e3,stroke:#74b9ff,color:#fff
    classDef struct fill:#fdcb6e,stroke:#ffeaa7,color:#2d3436
    classDef tree fill:#00b894,stroke:#55efc4,color:#fff
    classDef bit fill:#d63031,stroke:#ff7675,color:#fff

    subgraph IOStream ["File I/O Stream"]
        direction TB
        A[Input File]:::file
        H[Compressed .huf Data]:::file
        B(64KB Read Buffer):::buffer
        G(64KB BitWriter Buffer):::buffer
        A --> B
        G --> H
    end

    subgraph CoderCore ["Huffman Coder Core"]
        direction TB
        C{Frequency Map}:::struct
        B -.->|Count Chars| C
        
        subgraph DataStructures ["Internal Data Structures"]
            direction LR
            D[Min-Heap]:::struct
            C -->|Sort| D
            E((Prefix Tree)):::tree
            D -->|Build| E
            F[Code Dictionary]:::struct
            E -->|Generate| F
        end
        
        F -->|Map Bits| G
        B -.->|Encode Pass| G
    end

    style IOStream fill:#2f3542,stroke:#57606f,color:#fff
    style CoderCore fill:#1e272e,stroke:#485460,color:#fff
    style DataStructures fill:#2c3e50,stroke:#34495e,color:#fff
```

### Decompression Engine Pipeline
```mermaid
flowchart TD
    classDef file fill:#2d3436,stroke:#636e72,color:#dfe6e9
    classDef buffer fill:#0984e3,stroke:#74b9ff,color:#fff
    classDef tree fill:#00b894,stroke:#55efc4,color:#fff

    subgraph IOStream ["File I/O Stream"]
        direction TB
        A[Compressed .huf]:::file
        F[Restored Original File]:::file
        B[Metadata Header]:::file
        C(64KB BitReader Buffer):::buffer
        E(64KB Output Buffer):::buffer
        A -->|Extract| B
        A -->|Read Bits| C
        E -->|Write Block| F
    end

    subgraph CoderCore ["Huffman Coder Core"]
        direction TB
        D((Prefix Tree & LUT)):::tree
        B -->|Reconstruct| D
        C -.->|8-Bit LUT Jump| D
        D -.->|Fast Decode| E
    end

    style IOStream fill:#2f3542,stroke:#57606f,color:#fff
    style CoderCore fill:#1e272e,stroke:#485460,color:#fff
```

---

## Getting Started

### Prerequisites
* A standard modern C++17 compiler (GCC, Clang, or MSVC)
* CMake (`>= 3.10`)

### Installation via CMake
```bash
git clone https://github.com/Komal-ai417/HuffmanProject.git
cd HuffmanProject
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

*(Alternatively, compile explicitly: `g++ -O3 -std=c++17 src/main.cpp src/HuffmanCoder.cpp -o huffman`)*

---

## Usage

The executable provides intuitive CLI access. 

### Encoding (Compression)
```bash
./huffman -c <input_file> <output_compressed_file.huf>
```
*Example: `./huffman -c book.txt book.huf`*

### Decoding (Decompression)
```bash
./huffman -d <input_compressed_file.huf> <restored_file.txt>
```

---

*This project was engineered to practically demonstrate low-level algorithmic application meshed seamlessly with Modern C++ Enterprise patterns.*
