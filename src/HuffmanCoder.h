#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>

class HuffmanCoder {
public:
    /// Compresses the given inputFile and writes binary data to outputFile.
    void compress(const std::string& inputFile, const std::string& outputFile);

    /// Decompresses the given inputFile and writes the original text to outputFile.
    void decompress(const std::string& inputFile, const std::string& outputFile);

private:
    /// Huffman Tree Node
    struct Node {
        char ch;
        uint64_t freq;
        uint64_t id;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        Node(char character, uint64_t frequency, uint64_t nodeId) 
            : ch(character), freq(frequency), id(nodeId), left(nullptr), right(nullptr) {}

        Node(char character, uint64_t frequency, uint64_t nodeId, std::unique_ptr<Node> l, std::unique_ptr<Node> r)
            : ch(character), freq(frequency), id(nodeId), left(std::move(l)), right(std::move(r)) {}
    };

    /// Generates the Huffman code mapping based on the constructed prefix tree
    void buildCodes(const Node* root, const std::string& currentCode, std::unordered_map<char, std::string>& huffmanCodes);

    /// Builds the Huffman tree from character frequencies yielding the root unique pointer
    std::unique_ptr<Node> buildTree(const std::unordered_map<char, uint64_t>& frequencies);
};
