#pragma once

#include <string>
#include <array>
#include <vector>
#include <cstdint>

class HuffmanCoder {
public:
    /// Compresses the given inputFile and writes binary data to outputFile.
    void compress(const std::string& inputFile, const std::string& outputFile);

    /// Decompresses the given inputFile and writes the original text to outputFile.
    void decompress(const std::string& inputFile, const std::string& outputFile);

private:
    /// Arena-allocated flat tree node — uses integer indices instead of pointers.
    /// This keeps all nodes contiguous in memory for maximum CPU cache efficiency.
    struct Node {
        int left  = -1;  // Index into the tree vector for the left child (-1 = none)
        int right = -1;  // Index into the tree vector for the right child (-1 = none)
        uint64_t freq = 0;
        unsigned char ch = 0;

        Node() = default;
        Node(unsigned char character, uint64_t frequency)
            : ch(character), freq(frequency) {}
    };

    /// Integer-based code representation: bits are stored in a uint64_t bitmask
    /// and length tracks how many of those bits are valid.
    /// Eliminates all std::string overhead from the hot encoding path.
    struct Code {
        uint64_t bits  = 0;
        uint8_t  length = 0;
    };

    /// Recursively generates the Huffman code for each leaf character.
    void buildCodes(int nodeIndex, const std::vector<Node>& tree,
                    uint64_t currentBits, uint8_t currentLen,
                    std::array<Code, 256>& huffmanCodes);

    /// Builds the Huffman tree into 'tree' using arena allocation.
    /// Returns the root node index, or -1 for an empty input.
    int buildTree(const std::array<uint64_t, 256>& frequencies, std::vector<Node>& tree);
};
