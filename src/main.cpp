#include "project/HuffmanTrie.hpp"

#include <iostream>

int main() {
    const std::string text = "banana";
    std::unordered_map<char, std::size_t> frequencies;
    for (char character : text) {
        ++frequencies[character];
    }
    const auto tree = buildHuffmanTree(frequencies);
    const auto encoded = huffmanEncode(text, generateHuffmanCodes(tree));
    std::cout << "Encoded: " << encoded << '\n';
    std::cout << "Decoded: " << huffmanDecode(encoded, tree) << '\n';

    Trie trie;
    trie.insert("cat");
    trie.insert("car");
    trie.insert("dog");
    std::cout << "Words starting with ca:\n";
    for (const auto& word : trie.wordsWithPrefix("ca")) {
        std::cout << word << '\n';
    }
}
