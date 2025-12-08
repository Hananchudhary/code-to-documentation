#pragma once
#include<iostream>
#include"DiskManager.h"
using namespace std;
class BTree;

// ================================
// BTreeNode Class Implementation
// ================================
class BTreeNode {
private:
    bool isLeaf;
    int n;                     // Current number of keys
    int t;                     // Minimum degree (defines range for number of keys)
    vector<uint32_t> keys;     // Array of keys (sorted, max = 2t-1)
    vector<uint32_t> file_child; // Disk offsets for child nodes (max = 2t)
    vector<BTreeNode*> mem_child; // In-memory pointers to child nodes
    uint32_t diskidx;          // This node's own disk offset
    
    // Private helper methods
    int findKey(uint32_t k){
        int idx = 0;
        while (idx < n && keys[idx] < k) {
            idx++;
        }
        return idx;
    }
    void splitChild(int i, BTreeNode* y, BTree* tree){
        BTreeNode* z = new BTreeNode(y->t, y->isLeaf);
        z->n = t - 1;
        
        // Copy last (t-1) keys of y to z
        for (int j = 0; j < t-1; j++) {
            z->keys[j] = y->keys[j + t];
        }

        // Copy last t children of y to z
        if (!y->isLeaf) {
            for (int j = 0; j < t; j++) {
                z->mem_child[j] = y->mem_child[j + t];
                z->file_child[j] = y->file_child[j + t];
                y->mem_child[j + t] = nullptr;
                y->file_child[j + t] = 0;
            }
        }

        // Reduce keys in y
        y->n = t - 1;

        // Create space for new child in this node
        for (int j = n; j >= i+1; j--) {
            mem_child[j+1] = mem_child[j];
            file_child[j+1] = file_child[j];
        }

        // Link new child
        mem_child[i+1] = z;

        // Move keys to make space
        for (int j = n-1; j >= i; j--) {
            keys[j+1] = keys[j];
        }

        // Copy middle key of y to this node
        keys[i] = y->keys[t-1];

        // Increment key count
        n++;

        // Save all nodes
        tree->saveNode(y);
        tree->saveNode(z);
        tree->saveNode(this);
    }
    void insertNonFull(uint32_t k, BTree* tree){
        int i = n-1;
        
        if (isLeaf) {
            // Find position and move greater keys right
            while (i >= 0 && keys[i] > k) {
                keys[i+1] = keys[i];
                i--;
            }

            // Insert key
            keys[i+1] = k;
            n++;

            // Save to disk
            tree->saveNode(this);
        } else {
            // Find child for new key
            while (i >= 0 && keys[i] > k) {
                i--;
            }

            // Load child if needed
            if (mem_child[i+1] == nullptr && file_child[i+1] != 0) {
                mem_child[i+1] = tree->loadNode(file_child[i+1]);
            }

            // If child is full, split it
            if (mem_child[i+1] != nullptr && mem_child[i+1]->n == 2*t-1) {
                splitChild(i+1, mem_child[i+1], tree);

                // After split, decide which child gets new key
                if (keys[i+1] < k) {
                    i++;
                }
            }

            // Insert into appropriate child
            if (mem_child[i+1] != nullptr) {
                mem_child[i+1]->insertNonFull(k, tree);
            }
        }
    }
    void removeFromLeaf(int idx, BTree* tree){
        for (int i = idx+1; i < n; i++) {
            keys[i-1] = keys[i];
        }

        n--;
        tree->saveNode(this);
    }
    void removeFromNonLeaf(int idx, BTree* tree){
        uint32_t k = keys[idx];
        
        // Load children if needed
        if (mem_child[idx] == nullptr && file_child[idx] != 0) {
            mem_child[idx] = tree->loadNode(file_child[idx]);
        }
        if (mem_child[idx+1] == nullptr && file_child[idx+1] != 0) {
            mem_child[idx+1] = tree->loadNode(file_child[idx+1]);
        }

        if (mem_child[idx] != nullptr && mem_child[idx]->n >= t) {
            uint32_t pred = getPredecessor(idx, tree);
            keys[idx] = pred;
            mem_child[idx]->remove(pred, tree);
        } else if (mem_child[idx+1] != nullptr && mem_child[idx+1]->n >= t) {
            uint32_t succ = getSuccessor(idx, tree);
            keys[idx] = succ;
            mem_child[idx+1]->remove(succ, tree);
        } else {
            merge(idx, tree);
            if (mem_child[idx] != nullptr) {
                mem_child[idx]->remove(k, tree);
            }
        }
    }
    uint32_t getPredecessor(int idx, BTree* tree){
        // Load child if needed
        if (mem_child[idx] == nullptr && file_child[idx] != 0) {
            mem_child[idx] = tree->loadNode(file_child[idx]);
        }

        BTreeNode* cur = mem_child[idx];
        while (cur != nullptr && !cur->isLeaf) {
            // Load last child if needed
            int last = cur->n;
            if (cur->mem_child[last] == nullptr && cur->file_child[last] != 0) {
                cur->mem_child[last] = tree->loadNode(cur->file_child[last]);
            }
            cur = cur->mem_child[last];
        }

        return (cur != nullptr && cur->n > 0) ? cur->keys[cur->n-1] : 0;
    }
    uint32_t getSuccessor(int idx, BTree* tree){
        // Load child if needed
        if (mem_child[idx+1] == nullptr && file_child[idx+1] != 0) {
            mem_child[idx+1] = tree->loadNode(file_child[idx+1]);
        }

        BTreeNode* cur = mem_child[idx+1];
        while (cur != nullptr && !cur->isLeaf) {
            // Load first child if needed
            if (cur->mem_child[0] == nullptr && cur->file_child[0] != 0) {
                cur->mem_child[0] = tree->loadNode(cur->file_child[0]);
            }
            cur = cur->mem_child[0];
        }

        return (cur != nullptr && cur->n > 0) ? cur->keys[0] : 0;
    }
    void fill(int idx, BTree* tree){
        // Load siblings if needed
        if (idx != 0) {
            if (mem_child[idx-1] == nullptr && file_child[idx-1] != 0) {
                mem_child[idx-1] = tree->loadNode(file_child[idx-1]);
            }
        }
        if (idx != n) {
            if (mem_child[idx+1] == nullptr && file_child[idx+1] != 0) {
                mem_child[idx+1] = tree->loadNode(file_child[idx+1]);
            }
        }

        if (idx != 0 && mem_child[idx-1] != nullptr && mem_child[idx-1]->n >= t) {
            borrowFromPrev(idx, tree);
        } else if (idx != n && mem_child[idx+1] != nullptr && mem_child[idx+1]->n >= t) {
            borrowFromNext(idx, tree);
        } else {
            if (idx != n) {
                merge(idx, tree);
            } else {
                merge(idx-1, tree);
            }
        }
    }
    void borrowFromPrev(int idx, BTree* tree){
        BTreeNode* child = mem_child[idx];
        BTreeNode* sibling = mem_child[idx-1];
        
        // Move all keys in child one step ahead
        for (int i = child->n-1; i >= 0; i--) {
            child->keys[i+1] = child->keys[i];
        }

        // Move child pointers if not leaf
        if (!child->isLeaf) {
            for (int i = child->n; i >= 0; i--) {
                child->mem_child[i+1] = child->mem_child[i];
                child->file_child[i+1] = child->file_child[i];
            }
        }

        // Set child's first key
        child->keys[0] = keys[idx-1];

        // Move sibling's last child to child
        if (!child->isLeaf) {
            child->mem_child[0] = sibling->mem_child[sibling->n];
            child->file_child[0] = sibling->file_child[sibling->n];
        }

        // Move key from sibling to parent
        keys[idx-1] = sibling->keys[sibling->n-1];

        child->n++;
        sibling->n--;

        // Save nodes
        tree->saveNode(child);
        tree->saveNode(sibling);
        tree->saveNode(this);
    }
    void borrowFromNext(int idx, BTree* tree){
        BTreeNode* child = mem_child[idx];
        BTreeNode* sibling = mem_child[idx+1];
        
        // child's last key = parent's key
        child->keys[child->n] = keys[idx];
        
        // sibling's first child becomes child's last child
        if (!child->isLeaf) {
            child->mem_child[child->n+1] = sibling->mem_child[0];
            child->file_child[child->n+1] = sibling->file_child[0];
        }

        // sibling's first key becomes parent's key
        keys[idx] = sibling->keys[0];

        // Move all keys in sibling left
        for (int i = 1; i < sibling->n; i++) {
            sibling->keys[i-1] = sibling->keys[i];
        }

        // Move child pointers if not leaf
        if (!sibling->isLeaf) {
            for (int i = 1; i <= sibling->n; i++) {
                sibling->mem_child[i-1] = sibling->mem_child[i];
                sibling->file_child[i-1] = sibling->file_child[i];
            }
        }

        child->n++;
        sibling->n--;

        // Save nodes
        tree->saveNode(child);
        tree->saveNode(sibling);
        tree->saveNode(this);
    }
    void merge(int idx, BTree* tree){
        BTreeNode* child = mem_child[idx];
        BTreeNode* sibling = mem_child[idx+1];
        
        // Move key from parent to child
        child->keys[t-1] = keys[idx];
        
        // Copy keys from sibling
        for (int i = 0; i < sibling->n; i++) {
            child->keys[i+t] = sibling->keys[i];
        }

        // Copy children if not leaf
        if (!child->isLeaf) {
            for (int i = 0; i <= sibling->n; i++) {
                child->mem_child[i+t] = sibling->mem_child[i];
                child->file_child[i+t] = sibling->file_child[i];
            }
        }

        // Move keys in parent
        for (int i = idx+1; i < n; i++) {
            keys[i-1] = keys[i];
        }

        // Move child pointers
        for (int i = idx+2; i <= n; i++) {
            mem_child[i-1] = mem_child[i];
            file_child[i-1] = file_child[i];
        }

        child->n += sibling->n + 1;
        n--;

        // Free sibling
        delete sibling;
        mem_child[idx+1] = nullptr;
        file_child[idx+1] = 0;

        // Save nodes
        tree->saveNode(child);
        tree->saveNode(this);
    }
    
public:
    // Constructor
    BTreeNode(int _t = 4, bool leaf = true) 
        : t(_t), isLeaf(leaf), n(0), diskidx(0) {
        keys.resize(2 * t - 1, 0);
        file_child.resize(2 * t, 0);
        mem_child.resize(2 * t, nullptr);
    }
    
    // Destructor
    ~BTreeNode() {
        for (auto& child : mem_child) {
            if (child != nullptr) {
                delete child;
            }
        }
    }
    // Remove a key from the subtree rooted with this node
    void remove(uint32_t k, BTree* tree) {
        int idx = findKey(k);

        // The key to be removed is present in this node
        if (idx < n && keys[idx] == k) {
            // If the node is a leaf node
            if (isLeaf) {
                removeFromLeaf(idx, tree);
            } else {
                removeFromNonLeaf(idx, tree);
            }
        } else {
            // If this node is a leaf, then the key is not present in tree
            if (isLeaf) {
                cout << "Key " << k << " does not exist in the tree\n";
                return;
            }

            // The key to be removed is in the subtree rooted with this node
            // flag indicates whether the key is in the subtree rooted with 
            // the last child of this node
            bool flag = (idx == n);

            // Load child if needed
            if (mem_child[idx] == nullptr && file_child[idx] != 0) {
                mem_child[idx] = tree->loadNode(file_child[idx]);
            }

            // If the child where the key is supposed to exist has less than t keys,
            // fill that child
            if (mem_child[idx] != nullptr && mem_child[idx]->n < t) {
                fill(idx, tree);
            }

            // After fill, the tree might have changed
            // Reload current node from disk if needed
            // (In practice, you might need to reload due to splits/merges)

            // If the last child has been merged, it must have merged with the previous child
            // So we recurse on the (idx-1)th child. Else, we recurse on the idxth child
            // which now has at least t keys
            if (flag && idx > n) {
                // Load child[idx-1] if needed
                if (mem_child[idx-1] == nullptr && file_child[idx-1] != 0) {
                    mem_child[idx-1] = tree->loadNode(file_child[idx-1]);
                }
                if (mem_child[idx-1] != nullptr) {
                    mem_child[idx-1]->remove(k, tree);
                }
            } else {
                // Load child[idx] if needed (might have changed after fill)
                if (mem_child[idx] == nullptr && file_child[idx] != 0) {
                    mem_child[idx] = tree->loadNode(file_child[idx]);
                }
                if (mem_child[idx] != nullptr) {
                    mem_child[idx]->remove(k, tree);
                }
            }
        }
    }
    
    // Getters
    bool getIsLeaf() const { return isLeaf; }
    int getKeyCount() const { return n; }
    int getMinDegree() const { return t; }
    uint32_t getKey(int index) const { 
        return (index >= 0 && index < n) ? keys[index] : 0; 
    }
    uint32_t getDiskIdx() const { return diskidx; }
    
    // Setters
    void setDiskIdx(uint32_t idx) { diskidx = idx; }
    void setKey(int index, uint32_t key) { 
        if (index >= 0 && index < 2*t-1) keys[index] = key; 
    }
    void setFileChild(int index, uint32_t offset) { 
        if (index >= 0 && index < 2*t) file_child[index] = offset; 
    }
    void setMemChild(int index, BTreeNode* child) { 
        if (index >= 0 && index < 2*t) mem_child[index] = child; 
    }
    void setKeyCount(int count) { n = count; }
    void setIsLeaf(bool leaf) { isLeaf = leaf; }
    
    // Public methods
    BTreeNode* search(uint32_t k, BTree* tree){
        int i = 0;
        while (i < n && k > keys[i]) {
            i++;
        }

        // If found
        if (i < n && keys[i] == k) {
            return this;
        }

        // If leaf and not found
        if (isLeaf) {
            return nullptr;
        }

        // Load child if needed
        if (mem_child[i] == nullptr && file_child[i] != 0) {
            mem_child[i] = tree->loadNode(file_child[i]);
        }

        // Search in child
        return (mem_child[i] != nullptr) ? mem_child[i]->search(k, tree) : nullptr;
    }
    void traverse(){
        cout << "[";
        for (int i = 0; i < n; i++) {
            cout << keys[i];
            if (i < n-1) cout << " ";
        }
        cout << "] ";

        if (!isLeaf) {
            for (int i = 0; i <= n; i++) {
                if (mem_child[i] != nullptr) {
                    mem_child[i]->traverse();
                }
            }
        }
    }
    
    // Friend class declaration
    friend class BTree;
};

class BTree {
private:
    BTreeNode* root;
    DiskManager<BTreeNode> disk;
    int t;  // Minimum degree
    
    // Load a node from disk if not in memory
    
    
    // Recursive delete subtree
    void deleteSubtree(BTreeNode* node) {
        if (node == nullptr) return;
        
        if (!node->isLeaf) {
            for (int i = 0; i <= node->n; i++) {
                if (node->mem_child[i] != nullptr) {
                    deleteSubtree(node->mem_child[i]);
                }
            }
        }
        
        delete node;
    }
    
public:
BTreeNode* loadNode(uint32_t offset) {
        if (offset == 0) return nullptr;
        
        // Check if already loaded (simple implementation)
        // In advanced version, use LRU cache
        return &disk.readNode(offset);
    }
    
    // Save a node to disk
    void saveNode(BTreeNode* node) {
        if (node == nullptr) return;
        disk.writeNode(node->diskidx, *node);
    }
    // Constructor
    BTree(const string& filename, int _t = 4) 
        : disk(filename), t(_t), root(nullptr) {
        
        // Try to load root from existing file
        loadFromFile();
    }
    
    // Destructor
    ~BTree() {
        saveToFile();
        deleteSubtree(root);
    }
    
    // Public interface methods
    
    // Search for a key
    bool search(uint32_t k) {
        if (root == nullptr) return false;
        
        BTreeNode* result = root->search(k, this);
        return (result != nullptr);
    }
    
    // Insert a key
    void insert(uint32_t k) {
        // If tree is empty
        if (root == nullptr) {
            root = new BTreeNode(t, true);
            root->keys[0] = k;
            root->n = 1;
            saveNode(root);
        } else {
            // If root is full, tree grows in height
            if (root->n == 2*t - 1) {
                BTreeNode* newRoot = new BTreeNode(t, false);
                newRoot->setMemChild(0, root);
                newRoot->splitChild(0, root, this);
                
                // Decide which child will have new key
                int i = 0;
                if (newRoot->keys[0] < k) {
                    i++;
                }
                newRoot->mem_child[i]->insertNonFull(k, this);
                
                root = newRoot;
                saveNode(root);
            } else {
                root->insertNonFull(k, this);
            }
        }
    }
    
    // Remove a key
    // Remove a key from the B-Tree
    void BTree::remove(uint32_t k) {
        if (root == nullptr) {
            cout << "Tree is empty\n";
            return;
        }
        
        // Call remove for root
        root->remove(k, this);
        
        // If root becomes empty after deletion
        if (root->n == 0) {
            BTreeNode* temp = root;
            
            if (root->isLeaf) {
                root = nullptr;
            } else {
                // Load first child if needed
                if (root->mem_child[0] == nullptr && root->file_child[0] != 0) {
                    root->mem_child[0] = loadNode(root->file_child[0]);
                }
                root = root->mem_child[0];
            }
            
            // Delete old root
            delete temp;
            
            // Save new root
            if (root != nullptr) {
                saveNode(root);
            }
        }
    }
    
    // Traverse tree (for debugging)
    void traverse() {
        if (root != nullptr) {
            root->traverse();
        }
    }
    
    // Save entire tree to file
    void saveToFile() {
        if (root == nullptr) return;
        
        // Save all nodes recursively
        saveSubtree(root);
    }
    
    // Load tree from file
    void loadFromFile() {
        // Simple implementation: always read root from offset 4096
        // In real implementation, store root offset in file header
        uint32_t rootOffset = 4096; // After file header
        
        root = loadNode(rootOffset);
        if (root != nullptr) {
            // Load children recursively
            loadChildren(root);
        }
    }
    
private:
    // Recursive save
    void saveSubtree(BTreeNode* node) {
        if (node == nullptr) return;
        
        // Save this node
        saveNode(node);
        
        // Save children if not leaf
        if (!node->isLeaf) {
            for (int i = 0; i <= node->n; i++) {
                if (node->mem_child[i] != nullptr) {
                    saveSubtree(node->mem_child[i]);
                }
            }
        }
    }
    
    // Recursive load
    void loadChildren(BTreeNode* node) {
        if (node == nullptr || node->isLeaf) return;
        
        for (int i = 0; i <= node->n; i++) {
            if (node->file_child[i] != 0) {
                node->mem_child[i] = loadNode(node->file_child[i]);
                if (node->mem_child[i] != nullptr) {
                    loadChildren(node->mem_child[i]);
                }
            }
        }
    }
};