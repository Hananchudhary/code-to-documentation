#ifndef SAVL_H
#define SAVL_H
#include<iostream>
#include<vector>
#include<string>
#include"stack.h"
using namespace std;
template<typename T>
struct sAVLNode {
    std::string key;
    T value;
    int height;
    sAVLNode* left;
    sAVLNode* right;
    
    sAVLNode(const std::string& k, const T& v) 
        : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
};

template<typename T>
class sAVLTree {
private:
    sAVLNode<T>* root;
    int _size;
    int getHeight(sAVLNode<T>* node) {
        return node ? node->height : 0;
    }
    
    int getBalance(sAVLNode<T>* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }
    
    void updateHeight(sAVLNode<T>* node) {
        if (node) {
            node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        }
    }
    
    sAVLNode<T>* rotateRight(sAVLNode<T>* y) {
        sAVLNode<T>* x = y->left;
        sAVLNode<T>* T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        updateHeight(y);
        updateHeight(x);
        
        return x;
    }
    
    sAVLNode<T>* rotateLeft(sAVLNode<T>* x) {
        sAVLNode<T>* y = x->right;
        sAVLNode<T>* T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        updateHeight(x);
        updateHeight(y);
        
        return y;
    }
    
    sAVLNode<T>* insertNode(sAVLNode<T>*& node, const std::string& key, const T& value) {
        if (!node){ 
            return new sAVLNode<T>(key, value);
            _size++;
        }
        
        if (key < node->key) {
            node->left = insertNode(node->left, key, value);
        } else if (key > node->key) {
            node->right = insertNode(node->right, key, value);
        } else {
            node->value = value;
            return node;
        }
        
        updateHeight(node);
        int balance = getBalance(node);
        
        // Left-Left
        if (balance > 1 && key < node->left->key) {
            return rotateRight(node);
        }
        
        // Right-Right
        if (balance < -1 && key > node->right->key) {
            return rotateLeft(node);
        }
        
        // Left-Right
        if (balance > 1 && key > node->left->key) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        
        // Right-Left
        if (balance < -1 && key < node->right->key) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        
        return node;
    }
    
    sAVLNode<T>* findMin(sAVLNode<T>* node) {
        while (node->left) node = node->left;
        return node;
    }
    
    sAVLNode<T>* deleteNode(sAVLNode<T>* node, const std::string& key) {
        if (!node) return nullptr;
        
        if (key < node->key) {
            node->left = deleteNode(node->left, key);
        } else if (key > node->key) {
            node->right = deleteNode(node->right, key);
        } else {
            if (!node->left || !node->right) {
                sAVLNode<T>* temp = node->left ? node->left : node->right;
                if (!temp) {
                    temp = node;
                    node = nullptr;
                } else {
                    *node = *temp;
                }
                delete temp;
            } else {
                sAVLNode<T>* temp = findMin(node->right);
                node->key = temp->key;
                node->value = temp->value;
                node->right = deleteNode(node->right, temp->key);
            }
        }
        
        if (!node) return nullptr;
        
        updateHeight(node);
        int balance = getBalance(node);
        
        // Left-Left
        if (balance > 1 && getBalance(node->left) >= 0) {
            return rotateRight(node);
        }
        
        // Left-Right
        if (balance > 1 && getBalance(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        
        // Right-Right
        if (balance < -1 && getBalance(node->right) <= 0) {
            return rotateLeft(node);
        }
        
        // Right-Left
        if (balance < -1 && getBalance(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        
        return node;
    }
    
    sAVLNode<T>* searchNode(sAVLNode<T>* root, const std::string& key)
    {
        if (!root) return nullptr;

        Stack<sAVLNode<T>*> st;
        st.push(root);

        while (!st.empty()) {
            sAVLNode<T>* node = st.top();
            st.pop();

            if (!node) continue;

            if (node->key == key)
                return node;

            if (key < node->key)
                st.push(node->left);
            else
                st.push(node->right);
        }

        return nullptr;
    }
    
    void collectAll(sAVLNode<T>* node, std::vector<T>& result) {
        if (!node) return;
        collectAll(node->left, result);
        result.push_back(node->value);
        collectAll(node->right, result);
    }
    void destroyTree(sAVLNode<T>* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
    void collectKeys(sAVLNode<T>* node, vector<string>& res){
        if(!node) return;
        collectKeys(node->left, res);
        res.push_back(node->key);
        collectKeys(node->right, res); 
    }
    
public:
    sAVLTree() : root(nullptr) {}
    int size(){return this->_size;}
    ~sAVLTree() {
        destroyTree(root);
    }
    
    sAVLNode<T>* search(const std::string& key) {
        return searchNode(root, key);
    }
    void insert(const std::string& key, const T& value) {
        root = insertNode(root, key, value);
    }
    
    bool find(const std::string& key, T& value) {
        sAVLNode<T>* node = searchNode(root, key);
        if (node) {
            value = node->value;
            return true;
        }
        return false;
    }
    
    bool remove(const std::string& key) {
        if (!searchNode(root, key)) return false;
        root = deleteNode(root, key);
        return true;
    }
    
    std::vector<T> getAllValues() {
        std::vector<T> result;
        collectAll(root, result);
        return result;
    }
    vector<string> getAllKeys(){
        vector<string> res;
        collectKeys(root, res);
        return res;
    }
    bool exists(const std::string& key) {
        return searchNode(root, key) != nullptr;
    }
    
};
#endif