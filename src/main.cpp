#include "HuffmanCoder.h"
#include <iostream>
#include <string>

void printHelp() {
    std::cout << "Huffman Compression Utility\n"
              << "High-performance lossless file compressor using Huffman Coding.\n"
              << "Optimised with arena-allocated trees, fixed-array frequency tables,\n"
              << "64KB I/O buffering, and integer bitwise code emission.\n\n"
              << "Usage:\n"
              << "  Compress:   huffman -c <input_file>  <output_file.huf>\n"
              << "  Decompress: huffman -d <input_file.huf> <output_file>\n\n"
              << "Examples:\n"
              << "  huffman -c document.txt  document.huf\n"
              << "  huffman -d document.huf  document_restored.txt\n"
              << "  huffman -c image.png     image.huf\n";
}

int main(int argc, char* argv[]) {
    // Assert 4 active params are provided
    if (argc != 4) {
        std::cerr << "Invalid parameters executed.\n\n";
        printHelp();
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFile = argv[2];
    std::string outputFile = argv[3];

    HuffmanCoder coder;

    try {
        if (mode == "-c") {
            std::cout << "[INFO] Compressing '" << inputFile << "' -> '" << outputFile << "'\n";
            coder.compress(inputFile, outputFile);
            std::cout << "[SUCCESS] File compressed seamlessly.\n";
        } else if (mode == "-d") {
            std::cout << "[INFO] Decompressing '" << inputFile << "' -> '" << outputFile << "'\n";
            coder.decompress(inputFile, outputFile);
            std::cout << "[SUCCESS] File fully mathematically reconstructed.\n";
        } else {
            std::cerr << "Mode validation failed. Ensure using either -c or -d.\n\n";
            printHelp();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL EXCEPTION RUNTIME ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
