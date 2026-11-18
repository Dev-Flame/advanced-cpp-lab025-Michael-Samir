#include "project/HuffmanTrie.hpp"

#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>

HuffmanNode::HuffmanNode(char ch, std::size_t freq)
    : character(ch), frequency(freq) {
}

HuffmanNode::HuffmanNode(
    std::size_t freq,
    std::shared_ptr<HuffmanNode> leftChild,
    std::shared_ptr<HuffmanNode> rightChild)
    : character('\0'),
      frequency(freq),
      left(std::move(leftChild)),
      right(std::move(rightChild)) {
}

bool HuffmanNode::isLeaf() const {
    return left == nullptr && right == nullptr;
}

std::shared_ptr<HuffmanNode> buildHuffmanTree(
    const std::unordered_map<char, std::size_t>& frequencies) {
    using NodePtr = std::shared_ptr<HuffmanNode>;
    const auto compare = [](const NodePtr& first, const NodePtr& second) {
        return first->frequency > second->frequency;
    };
    std::priority_queue<NodePtr, std::vector<NodePtr>, decltype(compare)> nodes(compare);
    for (const auto& [character, frequency] : frequencies) {
        nodes.push(std::make_shared<HuffmanNode>(character, frequency));
    }

    // Each merge gives the two least-frequent nodes a common parent.
    while (nodes.size() > 1) {
        auto left = nodes.top();
        nodes.pop();
        auto right = nodes.top();
        nodes.pop();
        if (right->frequency > std::numeric_limits<std::size_t>::max() - left->frequency) {
            throw std::overflow_error("Huffman frequency sum is too large");
        }
        nodes.push(std::make_shared<HuffmanNode>(left->frequency + right->frequency, left, right));
    }
    return nodes.empty() ? nullptr : nodes.top();
}

namespace {

void collectCodes(
    const HuffmanNode* node,
    std::string& path,
    std::unordered_map<char, std::string>& codes) {
    if (!node) {
        return;
    }
    if (node->isLeaf()) {
        codes[node->character] = path.empty() ? "0" : path;
        return;
    }

    path.push_back('0');
    collectCodes(node->left.get(), path, codes);
    path.back() = '1';
    collectCodes(node->right.get(), path, codes);
    path.pop_back();
}

} // namespace

std::unordered_map<char, std::string> generateHuffmanCodes(
    const std::shared_ptr<HuffmanNode>& root) {
    std::unordered_map<char, std::string> codes;
    std::string path;
    collectCodes(root.get(), path, codes);
    return codes;
}

std::string huffmanEncode(
    const std::string& text,
    const std::unordered_map<char, std::string>& codes) {
    std::string encoded;
    for (char character : text) {
        const auto code = codes.find(character);
        if (code == codes.end()) {
            throw std::invalid_argument("No Huffman code for input character");
        }
        encoded += code->second;
    }
    return encoded;
}

std::string huffmanDecode(
    const std::string& encoded,
    const std::shared_ptr<HuffmanNode>& root) {
    if (encoded.empty()) {
        return {};
    }
    if (!root) {
        throw std::invalid_argument("Cannot decode bits without a Huffman tree");
    }
    if (root->isLeaf()) {
        if (encoded.find_first_not_of('0') != std::string::npos) {
            throw std::invalid_argument("A single-character tree uses only 0 bits");
        }
        return std::string(encoded.size(), root->character);
    }

    std::string decoded;
    const HuffmanNode* current = root.get();
    for (char bit : encoded) {
        if (bit == '0') {
            current = current->left.get();
        } else if (bit == '1') {
            current = current->right.get();
        } else {
            throw std::invalid_argument("Huffman encoding must contain only 0 and 1");
        }
        if (!current) {
            throw std::invalid_argument("Bit sequence does not match the Huffman tree");
        }
        if (current->isLeaf()) {
            decoded += current->character;
            current = root.get();
        }
    }
    if (current != root.get()) {
        throw std::invalid_argument("Incomplete Huffman code at end of input");
    }
    return decoded;
}

Trie::Trie()
    : root(std::make_unique<TrieNode>()) {
}

void Trie::insert(const std::string& word) {
    if (!root) {
        root = std::make_unique<TrieNode>();
    }
    TrieNode* current = root.get();
    for (char character : word) {
        auto child = current->children.find(character);
        if (child == current->children.end()) {
            child = current->children.emplace(character, std::make_unique<TrieNode>()).first;
        }
        current = child->second.get();
    }
    current->isWord = true;
}

const Trie::TrieNode* Trie::findNode(const std::string& prefix) const {
    const TrieNode* current = root.get();
    for (char character : prefix) {
        if (!current) {
            return nullptr;
        }
        const auto child = current->children.find(character);
        if (child == current->children.end()) {
            return nullptr;
        }
        current = child->second.get();
    }
    return current;
}

bool Trie::contains(const std::string& word) const {
    const auto* node = findNode(word);
    return node && node->isWord;
}

bool Trie::startsWith(const std::string& prefix) const {
    const auto* node = findNode(prefix);
    return node && (node->isWord || !node->children.empty());
}

std::vector<std::string> Trie::wordsWithPrefix(
    const std::string& prefix) const {
    std::vector<std::string> results;
    if (const auto* node = findNode(prefix)) {
        auto current = prefix;
        collectWords(node, current, results);
    }
    return results;
}

void Trie::collectWords(
    const TrieNode* node,
    std::string& current,
    std::vector<std::string>& results) const {
    if (node->isWord) {
        results.push_back(current);
    }
    for (const auto& [character, child] : node->children) {
        current.push_back(character);
        collectWords(child.get(), current, results);
        current.pop_back();
    }
}
