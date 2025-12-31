#pragma once
#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Count frequency of each character in string
unordered_map<char, int> countfreq(const string& word){
    unordered_map<char, int> freq;
    for(char c : word){
        freq[c]++;
    }
    return freq;
}

// Huffman Node
struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// Comparator for priority queue (min-heap)
// Deterministic: smaller frequency first, then smaller char
struct CompareNode {
    bool operator()(HuffmanNode* a, HuffmanNode* b){
        if(a->freq != b->freq) return a->freq > b->freq;
        return a->ch > b->ch; // tie-breaker
    }
};

class HuffmanCoding {
private:
    HuffmanNode* root;

    void buildTree(const string& text){
        unordered_map<char,int> freq = countfreq(text);
        priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareNode> pq;

        for(auto& [c,f] : freq){
            pq.push(new HuffmanNode(c,f));
        }

        while(pq.size() > 1){
            HuffmanNode* a = pq.top(); pq.pop();
            HuffmanNode* b = pq.top(); pq.pop();

            HuffmanNode* parent = new HuffmanNode('$', a->freq + b->freq);
            parent->left = a;
            parent->right = b;

            pq.push(parent);
        }

        root = pq.top();
    }

    void generateCodes(HuffmanNode* node, const string& code, unordered_map<char,string>& codes){
        if(!node) return;
        if(!node->left && !node->right){
            codes[node->ch] = code;
            return;
        }
        generateCodes(node->left, code + '0', codes);
        generateCodes(node->right, code + '1', codes);
    }

    void freeTree(HuffmanNode* node){
        if(!node) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

public:
    HuffmanCoding(): root(nullptr) {}

    unordered_map<char,string> getEncodedMap(const string& text){
        buildTree(text);
        unordered_map<char,string> codes;
        generateCodes(root, "", codes);
        return codes;
    }

    string encode(const string& text){
        unordered_map<char,string> codes = getEncodedMap(text);
        string encoded;
        for(char c : text){
            encoded += codes[c];
        }
        return encoded;
    }

    string decode(const string& encoded){
        string decoded;
        HuffmanNode* node = root;
        for(char bit : encoded){
            node = (bit == '0') ? node->left : node->right;
            if(!node->left && !node->right){
                decoded += node->ch;
                node = root;
            }
        }
        return decoded;
    }

    ~HuffmanCoding(){
        freeTree(root);
    }
};
