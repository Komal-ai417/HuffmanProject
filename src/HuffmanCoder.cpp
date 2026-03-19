#include "HuffmanCoder.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <queue>

// =============================================================================
//  BUFFERED BIT I/O
// =============================================================================
namespace {

    /// High-performance bit writer that batches bits into a 64-bit integer
    /// and flushes to a 64KB byte buffer — minimising syscall overhead and
    /// branch predictions compared to a per-bit byte buffer.
    class BitWriter {
    public:
        static constexpr size_t BUFFER_SIZE = 65536; // 64 KB

        explicit BitWriter(std::ofstream& out)
            : outStream(out), bufferPos(0), bitBuffer(0), bitsInBuffer(0)
        {
            byteBuffer.resize(BUFFER_SIZE);
        }

        ~BitWriter() { flush(); }

        /// Writes 'length' bits from 'codeBits' (MSB first) in a single operation.
        /// Uses bitwise shifts — no per-character loop required.
        void writeCode(uint64_t codeBits, uint8_t length) {
            bitBuffer     = (bitBuffer << length) | codeBits;
            bitsInBuffer += length;

            while (bitsInBuffer >= 8) {
                bitsInBuffer -= 8;
                byteBuffer[bufferPos++] = static_cast<char>((bitBuffer >> bitsInBuffer) & 0xFF);
                if (bufferPos == BUFFER_SIZE) {
                    outStream.write(byteBuffer.data(), BUFFER_SIZE);
                    bufferPos = 0;
                }
            }
        }

        void flush() {
            if (bitsInBuffer > 0) {
                byteBuffer[bufferPos++] =
                    static_cast<char>((bitBuffer << (8 - bitsInBuffer)) & 0xFF);
                bitsInBuffer = 0;
            }
            if (bufferPos > 0) {
                outStream.write(byteBuffer.data(), static_cast<std::streamsize>(bufferPos));
                bufferPos = 0;
            }
        }

    private:
        std::ofstream&   outStream;
        std::vector<char> byteBuffer;
        size_t           bufferPos;
        uint64_t         bitBuffer;
        int              bitsInBuffer;
    };

    /// High-performance buffered bit reader that loads 64KB at a time.
    class BitReader {
    public:
        static constexpr size_t BUFFER_SIZE = 65536; // 64 KB

        explicit BitReader(std::ifstream& in)
            : inStream(in), bufferPos(0), bufferLimit(0), bitBuffer(0), bitsInBuffer(0)
        {
            buffer.resize(BUFFER_SIZE);
        }

        void fillBuffer() {
            while (bitsInBuffer <= 56) {
                if (bufferPos == bufferLimit) {
                    inStream.read(buffer.data(), BUFFER_SIZE);
                    bufferLimit = static_cast<size_t>(inStream.gcount());
                    bufferPos   = 0;
                    if (bufferLimit == 0) break; // True EOF
                }
                bitBuffer = (bitBuffer << 8) | static_cast<unsigned char>(buffer[bufferPos++]);
                bitsInBuffer += 8;
            }
        }

        /// Returns false only when the underlying stream is completely exhausted.
        bool readBit(bool& bit) {
            if (bitsInBuffer == 0) fillBuffer();
            if (bitsInBuffer == 0) return false;
            
            bit = (bitBuffer >> (bitsInBuffer - 1)) & 1;
            --bitsInBuffer;
            return true;
        }

        /// Peeks the next 8 bits (padded with 0s if near EOF)
        int peek8() {
            if (bitsInBuffer < 8) fillBuffer();
            if (bitsInBuffer >= 8) return (bitBuffer >> (bitsInBuffer - 8)) & 0xFF;
            if (bitsInBuffer > 0)  return (bitBuffer << (8 - bitsInBuffer)) & 0xFF;
            return 0;
        }

        /// Consumes n bits without reading them
        void consume(int n) {
            bitsInBuffer -= n;
        }

    private:
        std::ifstream&    inStream;
        std::vector<char> buffer;
        size_t            bufferPos;
        size_t            bufferLimit;
        uint64_t          bitBuffer;
        int               bitsInBuffer;
    };

} // end anonymous namespace


// =============================================================================
//  HUFFMAN TREE CONSTRUCTION
// =============================================================================

void HuffmanCoder::buildCodes(int nodeIndex, const std::vector<Node>& tree,
                               uint64_t currentBits, uint8_t currentLen,
                               std::array<Code, 256>& huffmanCodes) {
    if (nodeIndex == -1) return;

    const Node& node = tree[static_cast<size_t>(nodeIndex)];

    if (node.left == -1 && node.right == -1) {
        // Edge case: single unique character in the file
        huffmanCodes[node.ch] = (currentLen == 0) ? Code{0, 1} : Code{currentBits, currentLen};
        return;
    }

    if (currentLen >= 64) {
        throw std::runtime_error("Huffman tree depth exceeded 64 bits. This adversarial file structure is unsupported.");
    }

    buildCodes(node.left,  tree, (currentBits << 1),       static_cast<uint8_t>(currentLen + 1), huffmanCodes);
    buildCodes(node.right, tree, (currentBits << 1) | 1ULL, static_cast<uint8_t>(currentLen + 1), huffmanCodes);
}

int HuffmanCoder::buildTree(const std::array<uint64_t, 256>& frequencies, std::vector<Node>& tree) {
    tree.clear();
    tree.reserve(512); // Max nodes: 256 leaves + 255 internal = 511

    // Comparator on indices into 'tree' — deterministic on (freq, char)
    auto cmp = [&tree](int a, int b) {
        if (tree[static_cast<size_t>(a)].freq != tree[static_cast<size_t>(b)].freq)
            return tree[static_cast<size_t>(a)].freq > tree[static_cast<size_t>(b)].freq;
        return tree[static_cast<size_t>(a)].ch > tree[static_cast<size_t>(b)].ch; // stable tiebreak
    };

    std::vector<int> heap;
    heap.reserve(256);

    // Insert leaves in sorted character order — ensures deterministic traversal
    for (int i = 0; i < 256; ++i) {
        if (frequencies[static_cast<size_t>(i)] > 0) {
            int idx = static_cast<int>(tree.size());
            tree.emplace_back(static_cast<unsigned char>(i), frequencies[static_cast<size_t>(i)]);
            heap.push_back(idx);
        }
    }

    if (heap.empty()) return -1;

    std::make_heap(heap.begin(), heap.end(), cmp);

    while (heap.size() > 1) {
        std::pop_heap(heap.begin(), heap.end(), cmp);
        int left = heap.back(); heap.pop_back();

        std::pop_heap(heap.begin(), heap.end(), cmp);
        int right = heap.back(); heap.pop_back();

        // Internal node — character field unused, indices for children
        int parentIdx = static_cast<int>(tree.size());
        Node parent;
        parent.freq  = tree[static_cast<size_t>(left)].freq + tree[static_cast<size_t>(right)].freq;
        parent.left  = left;
        parent.right = right;
        tree.push_back(parent);

        heap.push_back(parentIdx);
        std::push_heap(heap.begin(), heap.end(), cmp);
    }

    return heap.front(); // Root index
}


// =============================================================================
//  COMPRESSION
// =============================================================================

void HuffmanCoder::compress(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in.is_open())
        throw std::runtime_error("Could not open input file for compression: " + inputFile);

    // --- Pass 1: Build frequency table ----------------------------------------
    // Fixed array O(1) lookup — replaces std::unordered_map entirely.
    std::array<uint64_t, 256> frequencies{};
    frequencies.fill(0);
    uint64_t totalChars = 0;

    constexpr size_t BUFFER_SIZE = 65536;
    std::vector<char> buffer(BUFFER_SIZE);

    // Correct EOF pattern: process exactly gcount() bytes per iteration.
    while (true) {
        in.read(buffer.data(), BUFFER_SIZE);
        auto count = in.gcount();
        if (count == 0) break;
        for (std::streamsize i = 0; i < count; ++i)
            frequencies[static_cast<unsigned char>(buffer[static_cast<size_t>(i)])]++;
        totalChars += static_cast<uint64_t>(count);
    }

    // --- Write compressed file header -----------------------------------------
    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open())
        throw std::runtime_error("Could not open output file for compression: " + outputFile);

    // Header format:
    //   [uint64_t  totalChars  ]  — number of characters in the original file
    //   [uint16_t  activeCount ]  — number of distinct characters (≤ 256)
    //   [activeCount × (uint8_t char, uint64_t freq)] — frequency table entries
    out.write(reinterpret_cast<const char*>(&totalChars), sizeof(totalChars));

    uint16_t activeCount = 0;
    for (uint64_t f : frequencies) if (f > 0) ++activeCount;
    out.write(reinterpret_cast<const char*>(&activeCount), sizeof(activeCount));

    for (int i = 0; i < 256; ++i) {
        if (frequencies[static_cast<size_t>(i)] > 0) {
            auto ch = static_cast<unsigned char>(i);
            out.write(reinterpret_cast<const char*>(&ch),                         sizeof(ch));
            out.write(reinterpret_cast<const char*>(&frequencies[static_cast<size_t>(i)]), sizeof(uint64_t));
        }
    }

    if (totalChars == 0) return; // Empty file — header is sufficient

    // --- Build tree & codes ---------------------------------------------------
    std::vector<Node> tree;
    int rootIdx = buildTree(frequencies, tree);

    std::array<Code, 256> huffmanCodes{};
    buildCodes(rootIdx, tree, 0, 0, huffmanCodes);

    // --- Pass 2: Encode bitstream ----------------------------------------------
    in.clear();
    in.seekg(0, std::ios::beg);
    BitWriter bitWriter(out);

    const Node& root = tree[static_cast<size_t>(rootIdx)];
    if (root.left != -1 || root.right != -1) { // Multi-character file
        while (true) {
            in.read(buffer.data(), BUFFER_SIZE);
            auto count = in.gcount();
            if (count == 0) break;
            for (std::streamsize i = 0; i < count; ++i) {
                const Code& code = huffmanCodes[static_cast<unsigned char>(buffer[static_cast<size_t>(i)])];
                bitWriter.writeCode(code.bits, code.length);
            }
        }
    }
    // BitWriter destructor flushes remaining bits
}


// =============================================================================
//  DECOMPRESSION
// =============================================================================

void HuffmanCoder::decompress(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    if (!in.is_open())
        throw std::runtime_error("Could not open input file for decompression: " + inputFile);

    // --- Read header ----------------------------------------------------------
    uint64_t totalChars = 0;
    if (!in.read(reinterpret_cast<char*>(&totalChars), sizeof(totalChars))) {
        std::ofstream out(outputFile, std::ios::binary); // Produce an empty output
        return;
    }

    uint16_t activeCount = 0;
    in.read(reinterpret_cast<char*>(&activeCount), sizeof(activeCount));

    std::array<uint64_t, 256> frequencies{};
    frequencies.fill(0);
    for (uint16_t i = 0; i < activeCount; ++i) {
        unsigned char ch = 0;
        uint64_t freq    = 0;
        in.read(reinterpret_cast<char*>(&ch),   sizeof(ch));
        in.read(reinterpret_cast<char*>(&freq),  sizeof(freq));
        frequencies[ch] = freq;
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open())
        throw std::runtime_error("Could not open output file for decompression: " + outputFile);

    if (totalChars == 0) return; // Empty file

    // --- Reconstruct tree (exact mirror of compress) --------------------------
    std::vector<Node> tree;
    int rootIdx = buildTree(frequencies, tree);

    // --- Build 8-bit Decompression Lookup Table (LUT) -------------------------
    struct LUTEntry {
        bool isLeaf;
        uint8_t bitsConsumed;
        unsigned char ch;
        int nodeIndex;
    };

    std::array<LUTEntry, 256> lut{};
    for (int i = 0; i < 256; ++i) {
        int curNode = rootIdx;
        int bits = 0;
        for (bits = 0; bits < 8; ++bits) {
            bool bit = (i >> (7 - bits)) & 1;
            curNode = bit ? tree[static_cast<size_t>(curNode)].right 
                          : tree[static_cast<size_t>(curNode)].left;
            
            if (tree[static_cast<size_t>(curNode)].left == -1 && 
                tree[static_cast<size_t>(curNode)].right == -1) {
                lut[static_cast<size_t>(i)] = {true, static_cast<uint8_t>(bits + 1), tree[static_cast<size_t>(curNode)].ch, 0};
                break;
            }
        }
        if (bits == 8) {
            lut[static_cast<size_t>(i)] = {false, 8, 0, curNode};
        }
    }

    // --- Decode bitstream with buffered output --------------------------------
    constexpr size_t OUT_BUFFER_SIZE = 65536;
    std::vector<char> outBuffer(OUT_BUFFER_SIZE);
    size_t outBufferPos = 0;

    const Node& root = tree[static_cast<size_t>(rootIdx)];

    // Fast-path: single unique character — no bitstream was written
    if (root.left == -1 && root.right == -1) {
        for (uint64_t i = 0; i < totalChars; ++i) {
            outBuffer[outBufferPos++] = static_cast<char>(root.ch);
            if (outBufferPos == OUT_BUFFER_SIZE) {
                out.write(outBuffer.data(), OUT_BUFFER_SIZE);
                outBufferPos = 0;
            }
        }
        if (outBufferPos > 0) out.write(outBuffer.data(), static_cast<std::streamsize>(outBufferPos));
        return;
    }

    BitReader bitReader(in);
    int       currentNode  = rootIdx;
    uint64_t  decodedChars = 0;

    while (decodedChars < totalChars) {
        // LUT Fast-Path if we are starting at the root
        if (currentNode == rootIdx) {
            int peekVal = bitReader.peek8();
            const auto& entry = lut[static_cast<size_t>(peekVal)];
            
            if (entry.isLeaf) {
                outBuffer[outBufferPos++] = static_cast<char>(entry.ch);
                if (outBufferPos == OUT_BUFFER_SIZE) {
                    out.write(outBuffer.data(), OUT_BUFFER_SIZE);
                    outBufferPos = 0;
                }
                bitReader.consume(entry.bitsConsumed);
                ++decodedChars;
                continue;
            } else {
                currentNode = entry.nodeIndex;
                bitReader.consume(8);
            }
        }

        // Fallback bit-by-bit traversal for deep tree branches
        bool bit;
        if (!bitReader.readBit(bit)) throw std::runtime_error("Unexpected EOF encountered during bitwise decompression parsing.");

        currentNode = bit ? tree[static_cast<size_t>(currentNode)].right : tree[static_cast<size_t>(currentNode)].left;

        const Node& cur = tree[static_cast<size_t>(currentNode)];
        if (cur.left == -1 && cur.right == -1) {
            outBuffer[outBufferPos++] = static_cast<char>(cur.ch);
            if (outBufferPos == OUT_BUFFER_SIZE) {
                out.write(outBuffer.data(), OUT_BUFFER_SIZE);
                outBufferPos = 0;
            }
            currentNode = rootIdx;
            ++decodedChars;
        }
    }

    if (outBufferPos > 0)
        out.write(outBuffer.data(), static_cast<std::streamsize>(outBufferPos));
}
