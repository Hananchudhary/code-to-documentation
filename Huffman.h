#pragma once
#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include"maxHeap.h"
#include <string.h>
#include<algorithm>
using namespace std;
int myHash(string key, int tableSize){
    int h = 0;
    int s = key.size();
    for(int i = 0;i<s;i++){
        h = (h * 31 + (key[i])) % tableSize;
    }
    return h;
}
unordered_map<char, int> countfreq(string word){
    int s = word.size();
    unordered_map<char, int> freq;
    for(int i = 0;i<s;i++){
        if(freq.count(word[i]) == 0){
            freq.insert({word[i], 1});
        }
        else{
            freq[word[i]]++;
        }
    }
    return freq;
}
struct HuffmanNode {
    char ch;                 // Character stored in the node (for leaf nodes)
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode() = default;
    HuffmanNode(char c, int f): ch{c}, freq{f}, left{nullptr}, right{nullptr} {}
    HuffmanNode(const HuffmanNode& obj){
        ch = obj.ch;
        freq = obj.freq;
        left = obj.left;
        right = obj.right;
    }
    bool operator<(const HuffmanNode& other) const{
        return this->freq < other.freq;
    }
    bool operator>(const HuffmanNode& other) const{
        return this->freq > other.freq;
    }
    bool operator<=(const HuffmanNode& other) const{
        return this->freq <= other.freq;
    }
    bool operator>=(const HuffmanNode& other) const{
        return this->freq >= other.freq;
    }
    bool operator==(const HuffmanNode& other) const{
        return this->freq == other.freq;
    }
    bool operator!=(const HuffmanNode& other) const{
        return this->freq != other.freq;
    }
};


class HuffmanCoding {
private:
    HuffmanNode* root;   // Root of Huffman Tree

    void buildTree(string word) {
        unordered_map<char, int> f = countfreq(word);
        // minHeap<HuffmanNode> m;
        priority_queue<HuffmanNode, vector<HuffmanNode>, greater<HuffmanNode>> m;
        int s = word.size();
        for (auto i : f){
            HuffmanNode n(i.first, i.second);
            m.push(n);
        }
        while(m.size() > 1){
            HuffmanNode* a = new HuffmanNode(m.top());
            m.pop();
            HuffmanNode* b = new HuffmanNode(m.top());
            m.pop();
            HuffmanNode* c = new HuffmanNode('$', a->freq+b->freq);
            // delete a;
            // delete b;
            if(a->freq < b->freq){
                c->left = a;
                c->right = b;
            }
            else{
                c->left = b;
                c->right = a;
            }
            m.push(*c);
        }
        HuffmanNode* n = new HuffmanNode(m.top());
        root = n;
        m.pop();
        // TODO:
        // 1. Create a min-heap of HuffmanNode using frequencies
        // 2. Insert all characters into the heap
        // 3. While heap size > 1:
        //    - Extract two nodes with smallest frequency
        //    - Create a new internal node with combined frequency
        //    - Attach extracted nodes as left and right children
        //    - Push the new node back into the heap
        // 4. Set 'root' as the remaining node in heap
    }

    // Generate Huffman codes by traversing the tree
    void generateCodes(HuffmanNode* node, string code, unordered_map<char, string>& huffmanCodes) {
        if(!node) return;
        if(!node->left && !node->right){
            huffmanCodes[node->ch] = code;
            return;
        }
        generateCodes(node->left, code + '0', huffmanCodes);
        generateCodes(node->right, code + '1', huffmanCodes);
        // TODO:
        // 1. Traverse tree recursively
        // 2. Append '0' for left traversal and '1' for right traversal
        // 3. When a leaf node is reached, store code for character
    }

    // Helper function to decode encoded string
    void decodeHelper(HuffmanNode* node, int& index,
                      string& encoded, string& decoded) {
        if(!node) return;
        if(!node->left && !node->right && node->ch != '$'){
            decoded += node->ch;
            return;
        }
        if(index >= encoded.size()) return;
        else if(encoded[index] == '0'){
            decodeHelper(node->left, ++index, encoded, decoded);
        }
        else if(encoded[index] == '1'){
            decodeHelper(node->right, ++index, encoded, decoded);
        }
        // TODO:
        // 1. Traverse tree according to bits in encoded string
        // 2. On reaching leaf node, append character to decoded string
        // 3. Reset traversal to root
    }

    // Free memory of Huffman Tree
    HuffmanNode* freeTree(HuffmanNode* node) {
        if(!node) return nullptr;
        node->left = freeTree(node->left);
        node->right = freeTree(node->right);
        delete node;
        return nullptr;
    }

public:
    HuffmanCoding(): root{nullptr} {}

    // Build Huffman codes from input text
    unordered_map<char, string> getEncodedMap(const string& text) {
        // TODO:
        // 1. Calculate frequency of each character
        // 2. Build Huffman Tree
        // 3. Generate Huffman codes
        // 4. Return character-to-code mapping
        this->buildTree(text);
        unordered_map<char, string> huff{};
        this->generateCodes(root, "", huff);
        return huff;
    }

    // Encode input text using Huffman codes
    string encode(const string& text) {
        string res;
        unordered_map<char, string> huff = getEncodedMap(text);
        int s = text.size();
        for(int i = 0;i<s;i++){
            res += huff[text[i]];
        }
        // TODO:
        // 1. Replace each character in text with its Huffman code
        // 2. Return encoded bit string
        return res;
    }

    // Decode encoded string using Huffman Tree
    string decode(string& encoded) {
        // TODO:
        // 1. Traverse Huffman Tree based on encoded bits
        // 2. Recover original string
        string text;
        int index = 0;
        while(index < encoded.size()){
            string ch;
            this->decodeHelper(root,index, encoded, ch);
            text+=ch;
        }
        return text;
    }

    ~HuffmanCoding() {
        root = freeTree(root);
        // TODO: Free entire Huffman Tree
    }
};
