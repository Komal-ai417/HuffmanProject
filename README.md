# Huffman Compression Algorithm 🗜️

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-CMake-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![CI](https://github.com/Komal-ai417/HuffmanProject/actions/workflows/ci.yml/badge.svg)

> **A high-performance, strictly memory-safe C++ command-line utility for compressing and decompressing arbitrary files using canonical Huffman Coding.**
>
> *Developed by [@Komal-ai417](https://github.com/Komal-ai417)*

---

## ✨ Technical Highlights

| Feature | Implementation |
|---|---|
| **Arena-Allocated Tree** | Huffman tree stored as a flat `std::vector<Node>` with integer child indices—all 511 nodes contiguous in memory, eliminating heap scatter and CPU cache misses. |
| **O(1) Frequency Table** | `std::array<uint64_t, 256>` replaces `std::unordered_map`—direct index lookup, zero hashing, and zero dynamic allocation. |
| **Integer Bit Codes** | Variable-length codes stored as `uint64_t bits` + `uint8_t length`—entire codewords emitted via a single bitwise shift, bypassing character-by-character string iteration. |
| **64KB Buffered I/O** | Both `BitWriter` and `BitReader` batch 64 KB at a time, minimizing system calls. Output decompression utilizes a matching 64KB flush buffer. |
| **Correct EOF Handling** | Read loop uses `while(true) { read; if(gcount==0) break; }`—preventing the subtle double-processing bug found in partial final chunks. |
| **Deterministic Trees** | Nodes inserted in sorted character order with frequency tie-breaking by character value—guaranteeing identical tree structure across platforms and compilers. |
| **Full Edge Case Safety** | Empty files (0 bytes), single repeated characters, and arbitrary binary data are all handled correctly with guaranteed round-trip fidelity. |

---

## ⚙️ System Architecture

### 📥 Compression Engine Pipeline
```mermaid
flowchart TD
    classDef file fill:#2d3436,stroke:#636e72,color:#dfe6e9
    classDef buffer fill:#0984e3,stroke:#74b9ff,color:#fff
    classDef struct fill:#fdcb6e,stroke:#ffeaa7,color:#2d3436
    classDef tree fill:#00b894,stroke:#55efc4,color:#fff

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

### 📤 Decompression Engine Pipeline
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
        D((Prefix Tree)):::tree
        B -->|Reconstruct| D
        C -.->|Bit Traversal| D
        D -.->|Decode Char| E
    end

    style IOStream fill:#2f3542,stroke:#57606f,color:#fff
    style CoderCore fill:#1e272e,stroke:#485460,color:#fff
```

---

## 🚀 Getting Started

### Prerequisites
- A standard modern C++17 compiler (GCC, Clang, or MSVC)
- CMake (`>= 3.10`)

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

## 💻 Usage

The executable provides intuitive CLI access.

### Encoding (Compression)
```bash
./huffman -c <input_file> <output_compressed_file.huf>
```
*Example: `./huffman -c document.txt document.huf`*

### Decoding (Decompression)
```bash
./huffman -d <input_compressed_file.huf> <restored_file.txt>
```
*Example: `./huffman -d document.huf document_restored.txt`*

---

## 🗂️ Project Structure

```
HuffmanProject/
├── .github/
│   └── workflows/
│       └── ci.yml          # Cross-platform CI (Ubuntu, Windows, macOS)
├── src/
│   ├── HuffmanCoder.h      # Class declaration, Node & Code structs
│   ├── HuffmanCoder.cpp    # Core compress/decompress implementation
│   └── main.cpp            # CLI entry point
├── tests/
│   └── test_huffman.cpp    # Automated C++ test suite
├── CMakeLists.txt
└── README.md
```

---

*This project was engineered to demonstrate low-level algorithmic design meshed seamlessly with modern C++ enterprise performance patterns.*
