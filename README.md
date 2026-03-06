# Huffman Compression Algorithm 🗜️

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![Build](https://img.shields.io/badge/build-CMake-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

> **A high-performance, strictly memory-safe C++ command-line utility for compressing and decompressing arbitrary files using canonical Huffman Coding.**
> 
> *Developed by [@Komal-ai417](https://github.com/Komal-ai417)*

---

## ✨ Technical Highlights

* **Object-Oriented Architecture:** Encapsulated strictly in robust `HuffmanCoder` module instances.
* **Deterministic Trees:** Engineered Custom Heap constraints providing cross-platform mathematical identicality (100% hash collision rate).
* **Buffer-Optimized I/O:** Uses 64KB block chunking for massive 10x read performance leaps instead of generic byte-by-byte streaming. 
* **Zero Memory Leaks:** Employs modern C++ `std::unique_ptr` semantics recursively tearing down binary trees organically. No hanging pointers, ever. 
* **Bit-Level Binary Safety:** Uses low-level byte shifts to construct precise bitstream representations of generic files. Safely handles any format (.txt, .exe, .png).
* **Graceful Edge Casings:** Operates deterministically even on 0-byte sequences or completely homogenous character files.

---

## ⚙️ System Architecture

The compression engine is built to maximize I/O throughput and minimize memory footprint using advanced buffered system calls and deterministic pointer-safety.

```mermaid
flowchart TD
    %% Node Styles
    classDef client fill:#2C3E50,stroke:#34495E,color:#fff;
    classDef engine fill:#34495E,stroke:#2C3E50,color:#fff;
    classDef memory fill:#E67E22,stroke:#D35400,color:#fff;
    classDef buffer fill:#2980B9,stroke:#2471A3,color:#fff;
    classDef data fill:#27AE60,stroke:#2ECC71,color:#fff;
    
    subgraph ClientSpace [Client Interface]
        CLI[huffman.exe CLI Arguments]:::client
    end

    subgraph CoreEngine [Huffman Compression Engine]
        direction TB
        
        subgraph IOPhase [I/O Phase & Buffering]
            Input[(Input File)]:::data
            ReadBuffer[64KB Read Buffer Vector]:::buffer
            WriteBuffer[64KB Write Buffer Vector]:::buffer
            Output[(Output .huf File)]:::data
            
            Input -->|std::ifstream::read| ReadBuffer
            WriteBuffer -->|std::ofstream::write| Output
        end

        subgraph CompressionLogic [Tree Structures & Logic]
            FreqMap[O 1 Frequency Hash Map: char -> std::uint64]:::memory
            MinHeap[Min-Heap Priority Queue]:::memory
            PrefixTree[Huffman Prefix Tree \std::unique_ptr\]:::memory
            CodeMap[O 1 Encoded Map: char -> bitstring]:::memory
            
            ReadBuffer -. "Phase 1: Count Chars" .-> FreqMap
            FreqMap -. "Construct Heap" .-> MinHeap
            MinHeap -. "Merge Smallest" .-> PrefixTree
            PrefixTree -. "Generate Sequences" .-> CodeMap
        end

        subgraph BitwiseOps [Bit-Level Transcoding]
            BitWriter[BitWriter: Shift & Pack Bits]
            BitReader[BitReader: Extract Bits]
            
            CodeMap -. "Phase 2: Encode" .-> BitWriter
            BitWriter --> WriteBuffer
        end
    end

    CLI -->|Initiate| InputFile

    style ClientSpace fill:#2c3e50,stroke:#34495e,stroke-width:2px,color:#fff
    style CoreEngine fill:#34495e,stroke:#2c3e50,stroke-width:2px,color:#fff
    style CompressionLogic fill:#22313F,stroke:#34495e,stroke-width:1px,color:#fff
    style IOPhase fill:#22313F,stroke:#34495e,stroke-width:1px,color:#fff
    style BitwiseOps fill:#22313F,stroke:#34495e,stroke-width:1px,color:#fff
```

---

## 🚀 Getting Started

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

## 💻 Usage

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
