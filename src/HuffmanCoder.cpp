#include "HuffmanCoder.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace {

    class BitWriter {
    public:
        static constexpr size_t BUFFER_SIZE = 65536;

        BitWriter(std::ofstream& out) : outStream(out), currentByte(0), bitCount(0), bufferPos(0) {
            buffer.resize(BUFFER_SIZE);
        }
        
        ~BitWriter() { 
            flush(); 
        }

        void writeBit(bool bit) {
            if (bit) {
                currentByte |= (1 << (7 - bitCount));
            }
            bitCount++;
            if (bitCount == 8) {
                buffer[bufferPos++] = currentByte;
                currentByte = 0;
                bitCount = 0;
                if (bufferPos == BUFFER_SIZE) {
                    outStream.write(buffer.data(), BUFFER_SIZE);
                    bufferPos = 0;
                }
            }
        }

        void flush() {
            if (bitCount > 0) {
                buffer[bufferPos++] = currentByte;
                bitCount = 0;
                currentByte = 0;
            }
            if (bufferPos > 0) {
                outStream.write(buffer.data(), bufferPos);
                bufferPos = 0;
            }
        }

    private:
        std::ofstream& outStream;
        unsigned char currentByte;
        int bitCount;
        std::vector<char> buffer;
        size_t bufferPos;
    };

    class BitReader {
    public:
        static constexpr size_t BUFFER_SIZE = 65536;

        BitReader(std::ifstream& in) : inStream(in), currentByte(0), bitCount(0), bufferPos(0), bufferLimit(0) {
            buffer.resize(BUFFER_SIZE);
        }

        // Returns true if a bit was successfully read
        bool readBit(bool& bit) {
            if (bitCount == 0) {
                if (bufferPos == bufferLimit) {
                    inStream.read(buffer.data(), BUFFER_SIZE);
                    bufferLimit = inStream.gcount();
                    bufferPos = 0;
                    if (bufferLimit == 0) return false;
                }
                currentByte = static_cast<unsigned char>(buffer[bufferPos++]);
                bitCount = 8;
            }
            bit = (currentByte & (1 << (bitCount - 1))) != 0;
            bitCount--;
            return true;
        }

    private:
        std::ifstream& inStream;
        unsigned char currentByte;
        int bitCount;
        std::vector<char> buffer;
        size_t bufferPos;
        size_t bufferLimit;
    };
} // end anonymous namespace

void HuffmanCoder::buildCodes(const Node* root, const std::string& currentCode, std::unordered_map<char, std::string>& huffmanCodes) {
    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCodes[root->ch] = currentCode.empty() ? "0" : currentCode; // Edge case: single character
        return;
    }

    buildCodes(root->left.get(), currentCode + "0", huffmanCodes);
    buildCodes(root->right.get(), currentCode + "1", huffmanCodes);
}

std::unique_ptr<HuffmanCoder::Node> HuffmanCoder::buildTree(const std::unordered_map<char, uint64_t>& frequencies) {
    if (frequencies.empty()) return nullptr;

    auto cmp = [](const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) {
        if (a->freq != b->freq) {
            return a->freq > b->freq; // min-heap configuration
        }
        return a->id > b->id; // Deterministic tie-breaker
    };

    // Sort structurally for determinism regardless of map iteration sequence
    std::vector<std::pair<char, uint64_t>> sortedFreqs(frequencies.begin(), frequencies.end());
    std::sort(sortedFreqs.begin(), sortedFreqs.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    std::vector<std::unique_ptr<Node>> heap;
    uint64_t currentId = 0;
    for (const auto& pair : sortedFreqs) {
        heap.push_back(std::make_unique<Node>(pair.first, pair.second, currentId++));
    }

    std::make_heap(heap.begin(), heap.end(), cmp);

    while (heap.size() > 1) {
        // Pop first smallest
        std::pop_heap(heap.begin(), heap.end(), cmp);
        auto left = std::move(heap.back());
        heap.pop_back();

        // Pop second smallest
        std::pop_heap(heap.begin(), heap.end(), cmp);
        auto right = std::move(heap.back());
        heap.pop_back();

        // Create parent node combining frequencies
        uint64_t sumFreq = left->freq + right->freq;
        auto parent = std::make_unique<Node>('\0', sumFreq, currentId++, std::move(left), std::move(right));

        // Push parent back to heap
        heap.push_back(std::move(parent));
        std::push_heap(heap.begin(), heap.end(), cmp);
    }

    return std::move(heap.front());
}

void HuffmanCoder::compress(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Could not open input file for compression: " + inputFile);
    }

    // 1. Calculate frequencies with a 64KB buffer
    std::unordered_map<char, uint64_t> frequencies;
    uint64_t totalChars = 0;
    
    constexpr size_t BUFFER_SIZE = 65536; // 64 KB
    std::vector<char> buffer(BUFFER_SIZE);

    while (in.read(buffer.data(), BUFFER_SIZE)) {
        size_t count = in.gcount();
        for (size_t i = 0; i < count; ++i) {
            frequencies[buffer[i]]++;
        }
        totalChars += count;
    }
    // Process remaining bytes if EOF is reached
    size_t count = in.gcount();
    for (size_t i = 0; i < count; ++i) {
        frequencies[buffer[i]]++;
    }
    totalChars += count;

    // Clear EOF flag and rewind for second pass
    in.clear();
    in.seekg(0, std::ios::beg);

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file for compression: " + outputFile);
    }

    // 2. Serialize header metadata
    // Format: [uint64_t totalChars] [size_t mapSize] [Key-Value Pairs of Map...]
    out.write(reinterpret_cast<const char*>(&totalChars), sizeof(totalChars));
    size_t mapSize = frequencies.size();
    out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    for (const auto& pair : frequencies) {
        out.write(&pair.first, sizeof(pair.first));
        out.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
    }

    if (totalChars == 0) {
        // Output file requires no further data; just standard header layout
        return; 
    }

    // 3. Build Tree & Generate Code map
    auto root = buildTree(frequencies);
    std::unordered_map<char, std::string> huffmanCodes;
    buildCodes(root.get(), "", huffmanCodes);

    // 4. Compress file bits with buffered reading
    BitWriter bitWriter(out);
    
    // Only parse standard bits if there's actual depth to the tree.
    if (root->left || root->right) {
        while (in.read(buffer.data(), BUFFER_SIZE)) {
            size_t count = in.gcount();
            for (size_t i = 0; i < count; ++i) {
                const std::string& code = huffmanCodes[buffer[i]];
                for (char bitChar : code) {
                    bitWriter.writeBit(bitChar == '1');
                }
            }
        }
        // Final block process
        size_t count = in.gcount();
        for (size_t i = 0; i < count; ++i) {
            const std::string& code = huffmanCodes[buffer[i]];
            for (char bitChar : code) {
                bitWriter.writeBit(bitChar == '1');
            }
        }
    }
}

void HuffmanCoder::decompress(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Could not open input file for decompression: " + inputFile);
    }

    // 1. Parse header data
    uint64_t totalChars = 0;
    if (!in.read(reinterpret_cast<char*>(&totalChars), sizeof(totalChars))) {
        // File is either entirely empty or malformed
        std::ofstream out(outputFile, std::ios::binary);
        return; 
    }

    size_t mapSize = 0;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    std::unordered_map<char, uint64_t> frequencies;
    for (size_t i = 0; i < mapSize; ++i) {
        char ch;
        uint64_t freq;
        in.read(&ch, sizeof(ch));
        in.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        frequencies[ch] = freq;
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file for decompression: " + outputFile);
    }

    if (totalChars == 0) {
        return; // Normal handling of gracefully empty files
    }

    // 2. Build reconstructive Prefix Tree
    auto root = buildTree(frequencies);

    // 3. Decompress sequentially parsing bits via prefix tree rules
    BitReader bitReader(in);
    Node* currentNode = root.get();
    uint64_t decodedChars = 0;

    constexpr size_t OUT_BUFFER_SIZE = 65536;
    std::vector<char> outBuffer(OUT_BUFFER_SIZE);
    size_t outBufferPos = 0;

    // Fast-path Edge Case: Entire payload is permutations of exactly *one* character. 
    // Thus ignoring variable bits as none were inherently necessary.
    if (!root->left && !root->right) {
        for (uint64_t i = 0; i < totalChars; ++i) {
            outBuffer[outBufferPos++] = root->ch;
            if (outBufferPos == OUT_BUFFER_SIZE) {
                out.write(outBuffer.data(), OUT_BUFFER_SIZE);
                outBufferPos = 0;
            }
        }
        if (outBufferPos > 0) {
            out.write(outBuffer.data(), outBufferPos);
        }
        return;
    }

    while (decodedChars < totalChars) {
        bool bit;
        if (!bitReader.readBit(bit)) {
            throw std::runtime_error("Unexpected EOF encountered during bitwise decompression parsing.");
        }

        if (bit) {
            currentNode = currentNode->right.get();
        } else {
            currentNode = currentNode->left.get();
        }

        // Output reconstructed byte if arriving safely down a unified leaf
        if (!currentNode->left && !currentNode->right) {
            outBuffer[outBufferPos++] = currentNode->ch;
            if (outBufferPos == OUT_BUFFER_SIZE) {
                out.write(outBuffer.data(), OUT_BUFFER_SIZE);
                outBufferPos = 0;
            }
            currentNode = root.get(); 
            decodedChars++;
        }
    }

    if (outBufferPos > 0) {
        out.write(outBuffer.data(), outBufferPos);
    }
}
