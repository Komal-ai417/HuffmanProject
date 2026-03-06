#include "HuffmanCoder.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>

bool compareFiles(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false; // File problem
    }

    if (f1.tellg() != f2.tellg()) {
        return false; // Size mismatch
    }

    // Seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

void runTest(const std::string& testName, const std::string& content) {
    std::cout << "[TEST] Running: " << testName << "...\n";
    std::string testFile = testName + "_input.txt";
    std::string compressedFile = testName + "_compressed.huf";
    std::string decompressedFile = testName + "_output.txt";

    {
        std::ofstream out(testFile, std::ios::binary);
        out << content;
    }

    HuffmanCoder coder;
    coder.compress(testFile, compressedFile);
    coder.decompress(compressedFile, decompressedFile);

    if (!compareFiles(testFile, decompressedFile)) {
        std::cerr << "[FAILED] " << testName << ": Files do not match.\n";
        exit(1);
    }
    std::cout << "[PASSED] " << testName << ": 100% Data Integrity.\n";
}

int main() {
    try {
        runTest("Standard_Text", "Hello Huffman! This is a standard test file verifying variable length encodings.");
        runTest("Empty_File", "");
        runTest("Single_Char", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        
        // large binary-like logic test
        std::string binaryBlob;
        for(int i=0; i<10000; i++) binaryBlob += (char)(i % 256);
        runTest("Binary_Blob", binaryBlob);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during testing: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\nAll test suites completed flawlessly.\n";
    return 0;
}
