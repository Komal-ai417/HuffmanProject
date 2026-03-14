#include "HuffmanCoder.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdio>  // std::remove

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

bool compareFiles(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) return false;          // File open error
    if (f1.tellg() != f2.tellg()) return false;        // Size mismatch

    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

// Deletes temp files created by a test run
void cleanupTest(const std::string& testName) {
    std::remove((testName + "_input.txt").c_str());
    std::remove((testName + "_compressed.huf").c_str());
    std::remove((testName + "_output.txt").c_str());
}

// ---------------------------------------------------------------------------
//  Test runner
// ---------------------------------------------------------------------------

void runTest(const std::string& testName, const std::string& content) {
    std::cout << "[TEST] Running: " << testName << "...\n";

    const std::string testFile        = testName + "_input.txt";
    const std::string compressedFile  = testName + "_compressed.huf";
    const std::string decompressedFile = testName + "_output.txt";

    // Write input data
    {
        std::ofstream out(testFile, std::ios::binary);
        out << content;
    }

    HuffmanCoder coder;
    coder.compress(testFile, compressedFile);
    coder.decompress(compressedFile, decompressedFile);

    if (!compareFiles(testFile, decompressedFile)) {
        std::cerr << "[FAILED] " << testName << ": Decompressed output does not match input.\n";
        cleanupTest(testName);
        std::exit(1);
    }

    std::cout << "[PASSED] " << testName << ": 100% Data Integrity.\n";
    cleanupTest(testName); // Always clean up after passing
}

// ---------------------------------------------------------------------------
//  Test suite
// ---------------------------------------------------------------------------

int main() {
    try {
        // Standard text with varied character frequencies
        runTest("Standard_Text",
                "Hello Huffman! This is a standard test file verifying variable length encodings.");

        // Edge case: completely empty file — header only, no bitstream
        runTest("Empty_File", "");

        // Edge case: single unique repeating character — no bitstream, fast-path decompression
        runTest("Single_Char",
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

        // Binary-like blob covering all 256 byte values — stress tests the full symbol table
        std::string binaryBlob;
        binaryBlob.reserve(10240);
        for (int i = 0; i < 10000; ++i)
            binaryBlob += static_cast<char>(static_cast<unsigned char>(i % 256));
        runTest("Binary_Blob", binaryBlob);

        // Highly compressible input — very skewed frequency distribution
        runTest("High_Entropy",
                std::string(5000, 'A') + std::string(2000, 'B') + std::string(500, 'C') +
                std::string(200, 'D') + std::string(50, 'E') + "XYZ!@#$");

    } catch (const std::exception& e) {
        std::cerr << "Exception during testing: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\nAll test suites completed flawlessly.\n";
    return 0;
}
