#ifndef AVL_H
#define AVL_H
#include<iostream>
#include<vector>
#include<string>
#include"stack.h"
using namespace std;
template<typename T>
class AVLNode {
    public:
    char key[MAX_USERNAME_LEN]{};
    int idx;
    T value;
    int height;
    AVLNode* left;
    AVLNode* right;
    uint32_t lchild;
    uint32_t rchild;
    AVLNode():height{-1}, left{nullptr}, right{nullptr}, lchild{0}, rchild{0}{}
    AVLNode(const std::string& k, const T& v, int i) 
        : value(v), height(1), left(nullptr),idx{i}, right(nullptr) {
            strncpy(key, k.c_str(), MAX_USERNAME_LEN);
        }
    ~AVLNode()=default;
};

template<typename T>
class AVLTree {
private:
    AVLNode<T>* root;
    int _size;
    int getHeight(AVLNode<T>* node) {
        return node ? node->height : 0;
    }
    
    int getBalance(AVLNode<T>* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }
    
    void updateHeight(AVLNode<T>* node) {
        if (node) {
            node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        }
    }
    
    AVLNode<T>* rotateRight(AVLNode<T>* y, fstream& file) {
        AVLNode<T>* x = y->left;
        AVLNode<T>* T2 = x->right;
        
        x->right = y;
        y->left = T2;
        
        updateHeight(y);
        updateHeight(x);
        file.seekp(y->idx, ios::beg);
        file.write(reinterpret_cast<const char*>(y), sizeof(*y));
        file.seekp(x->idx, ios::beg);
        file.write(reinterpret_cast<const char*>(x), sizeof(*x));
        return x;
    }
    
    AVLNode<T>* rotateLeft(AVLNode<T>* x, fstream& file) {
        AVLNode<T>* y = x->right;
        AVLNode<T>* T2 = y->left;
        
        y->left = x;
        x->right = T2;
        
        updateHeight(x);
        updateHeight(y);
        file.seekp(y->idx, ios::beg);
        file.write(reinterpret_cast<const char*>(y), sizeof(*y));
        file.seekp(x->idx, ios::beg);
        file.write(reinterpret_cast<const char*>(x), sizeof(*x));
        return y;
    }
    AVLNode<T>* insertNode(AVLNode<T>*& node, const std::string& key, const T& value,fstream& file, int idx) {
        if (!node){ 
            AVLNode<T>* t = new AVLNode<T>(key, value, idx);
            _size++;
            file.seekp(idx, ios::beg);
            file.write(reinterpret_cast<const char*>(t), sizeof(*t));
            return t;
        }
        
        if (strcmp(&key[0], node->key)<0) {
            node->left = insertNode(node->left, key, value, file, idx);
            if(node->left)
                node->lchild = node->left->idx;
        } else if (strcmp(&key[0], node->key)>0) {
            node->right = insertNode(node->right, key, value, file, idx);
            if(node->right)
                node->rchild = node->right->idx;
        } else {
            node->value = value;
            node->idx = idx;
            return node;
        }
        
        updateHeight(node);
        int balance = getBalance(node);
        
        // Left-Left
        if (balance > 1 && strcmp(&key[0], node->left->key)<0) {
            return rotateRight(node, file);
        }
        
        // Right-Right
        if (balance < -1 && strcmp(&key[0], node->right->key)>0) {
            return rotateLeft(node, file);
        }
        
        // Left-Right
        if (balance > 1 && strcmp(&key[0], node->left->key)>0) {
            node->left = rotateLeft(node->left, file);
            return rotateRight(node, file);
        }
        
        // Right-Left
        if (balance < -1 && strcmp(&key[0], node->right->key)<0) {
            node->right = rotateRight(node->right, file);
            return rotateLeft(node, file);
        }
        file.seekp(node->idx, ios::beg);
        file.write(reinterpret_cast<char*>(node), sizeof(*node));
        return node;
    }
    
    AVLNode<T>* findMin(AVLNode<T>* node) {
        while (node->left) node = node->left;
        return node;
    }
    
    AVLNode<T>* deleteNode(AVLNode<T>* node, const std::string& key, fstream& file) {
        if (!node) return nullptr;
        
        if (strcmp(key, node->key)<0) {
            node->left = deleteNode(node->left, key, file);
        } else if (strcmp(key, node->key)>0) {
            node->right = deleteNode(node->right, key, file);
        } else {
            if (!node->left || !node->right) {
                AVLNode<T>* temp = node->left ? node->left : node->right;
                if (!temp) {
                    temp = node;
                    node = nullptr;
                } else {
                    *node = *temp;
                }
                file.seekp(temp->idx, ios::beg);
                file.write(reinterpret_cast<const char*>('\0'), sizeof(*temp));
                delete temp;
                
            } else {
                AVLNode<T>* temp = findMin(node->right);
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
        file.seekp(node->idx, ios::beg);
        file.write(reinterpret_cast<const char*>(node), sizeof(*node));
        return node;
    }
    
    AVLNode<T>* searchNode(AVLNode<T>* root, const std::string& key)
    {
        if (!root) return nullptr;

        Stack<AVLNode<T>*> st;
        st.push(root);

        while (!st.empty()) {
            AVLNode<T>* node = st.top();
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
    
    void collectAll(AVLNode<T>* node, std::vector<T>& result) {
        if (!node) return;
        collectAll(node->left, result);
        result.push_back(node->value);
        collectAll(node->right, result);
    }
    void destroyTree(AVLNode<T>* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
    
    AVLNode<T>* load(AVLNode<T>* node,fstream& file, uint32_t i){
        if(i==0) return nullptr;
        file.seekg(i, ios::beg);
        AVLNode<T> u;
        file.read(reinterpret_cast<char*>(&u), sizeof(u));
        node = new AVLNode<T>(u);
        node->left = load(node->left, file, u.lchild);
        node->right = load(node->right, file, u.rchild);
        return node;
    }
    void getFiles(AVLNode<T>* node,const string& username, vector<string>& res){
        if(!node) return;
        if(strncmp(&node->key[0], &username[0], username.size()) == 0){
            res.push_back(node->value.name);
        }
        getFiles(node->left, username, res);
        getFiles(node->right, username, res);
    }
public:
    AVLTree() : root(nullptr){}
    int size(){return this->_size;}
    ~AVLTree() {
        destroyTree(root);
    }
    
    AVLNode<T>* search(const std::string& key) {
        return searchNode(root, key);
    }
    void insert(const std::string& key, const T& value,fstream& file, int idx) {
        root = insertNode(root, key, value,file,idx);
    }
    
    bool find(const std::string& key, T& value) {
        AVLNode<T>* node = searchNode(root, key);
        if (node) {
            value = node->value;
            return true;
        }
        return false;
    }
    
    bool remove(const std::string& key, fstream& file) {
        if (!searchNode(root, key)) return false;
        root = deleteNode(root, key,file);
        return true;
    }
    
    std::vector<T> getAllValues() {
        std::vector<T> result;
        collectAll(root, result);
        return result;
    }
    
    bool exists(const std::string& key) {
        return searchNode(root, key) != nullptr;
    }
    void loadTree(fstream& file,uint32_t idx){
        root = this->load(root, file, idx);
    }
    vector<string> getFiles(const string username){
        vector<string> res;
        getFiles(root, username, res);
        return res;
    }
};
#endif