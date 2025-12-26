#ifndef BTREE_H
#define BTREE_H
#include<iostream>
#include<math.h>
#include"DiskManager.h"
using namespace std;
template <int M,int K, int O>
class BTreeNode {
public:
    bool isLeaf;
    int n;                     // Current number of keys
    int t;                     // Minimum degree (defines range for number of keys)
    uint32_t keys[M]{};     // Array of keys (sorted, max = 2t-1)
    uint32_t file_child[M+1]{}; // Disk offsets for child nodes (max = 2t)
    BTreeNode<M, K,O>* mem_child[M+1]{}; // In-memory pointers to child nodes
    uint32_t diskidx;          // This node's own disk offset
    
    // Private helper methods
    int findKey(uint32_t k){
        int idx = 0;
        while (idx < n && keys[idx] < k) {
            idx++;
        }
        return idx;
    }
    void splitChild(int i, BTreeNode<M, K,O>* y, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        BTreeNode<M, K,O>* z = new BTreeNode<M, K,O>(y->isLeaf, disk.allocateBlock());
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
        disk.writeNode(y->diskidx,*y);
        disk.writeNode(z->diskidx,*z);
        disk.writeNode(this->diskidx,*this);
    }
    void insertNonFull(uint32_t k, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
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
            disk.writeNode(this->diskidx,*this);
        } else {
            // Find child for new key
            while (i >= 0 && keys[i] > k) {
                i--;
            }

            // Load child if needed
            if (mem_child[i+1] == nullptr && file_child[i+1] != 0) {
                BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[i+1]));
                mem_child[i+1] = b;
            }

            // If child is full, split it
            if (mem_child[i+1] != nullptr && mem_child[i+1]->n == 2*t-1) {
                splitChild(i+1, mem_child[i+1], disk);

                // After split, decide which child gets new key
                if (keys[i+1] < k) {
                    i++;
                }
            }

            // Insert into appropriate child
            if (mem_child[i+1] != nullptr) {
                mem_child[i+1]->insertNonFull(k, disk);
            }
        }
    }
    void removeFromLeaf(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        for (int i = idx+1; i < n; i++) {
            keys[i-1] = keys[i];
        }

        n--;
        disk.writeNode(this->diskidx,*this);
    }
    void removeFromNonLeaf(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        uint32_t k = keys[idx];
        
        // Load children if needed
        if (mem_child[idx] == nullptr && file_child[idx] != 0) {
            BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx]));
            mem_child[idx] = b;
        }
        if (mem_child[idx+1] == nullptr && file_child[idx+1] != 0) {
            BTreeNode<M, K,O>* b =new BTreeNode<M, K,O>(disk.readNode(file_child[idx+1]));
            mem_child[idx+1] = b;
        }

        if (mem_child[idx] != nullptr && mem_child[idx]->n >= t) {
            uint32_t pred = getPredecessor(idx, disk);
            keys[idx] = pred;
            mem_child[idx]->remove(pred, disk);
        } else if (mem_child[idx+1] != nullptr && mem_child[idx+1]->n >= t) {
            uint32_t succ = getSuccessor(idx, disk);
            keys[idx] = succ;
            mem_child[idx+1]->remove(succ, disk);
        } else {
            merge(idx, disk);
            if (mem_child[idx] != nullptr) {
                mem_child[idx]->remove(k, disk);
            }
        }
    }
    uint32_t getPredecessor(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        // Load child if needed
        if (mem_child[idx] == nullptr && file_child[idx] != 0) {
            BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx]));
            mem_child[idx] = b;
        }

        BTreeNode<M, K,O>* cur = mem_child[idx];
        while (cur != nullptr && !cur->isLeaf) {
            // Load last child if needed
            int last = cur->n;
            if (cur->mem_child[last] == nullptr && cur->file_child[last] != 0) {
                BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(cur->file_child[last]));
                cur->mem_child[last] = b;
            }
            cur = cur->mem_child[last];
        }

        return (cur != nullptr && cur->n > 0) ? cur->keys[cur->n-1] : 0;
    }
    uint32_t getSuccessor(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        // Load child if needed
        if (mem_child[idx+1] == nullptr && file_child[idx+1] != 0) {
            BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx+1]));
            mem_child[idx+1] = b;
        }

        BTreeNode<M, K,O>* cur = mem_child[idx+1];
        while (cur != nullptr && !cur->isLeaf) {
            // Load first child if needed
            if (cur->mem_child[0] == nullptr && cur->file_child[0] != 0) {
                BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(cur->file_child[0]));
                cur->mem_child[0] = b;
            }
            cur = cur->mem_child[0];
        }

        return (cur != nullptr && cur->n > 0) ? cur->keys[0] : 0;
    }
    void fill(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        // Load siblings if needed
        if (idx != 0) {
            if (mem_child[idx-1] == nullptr && file_child[idx-1] != 0) {
                BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx-1]));
                mem_child[idx-1] = b;
            }
        }
        if (idx != n) {
            if (mem_child[idx+1] == nullptr && file_child[idx+1] != 0) {
                BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx+1]));
                mem_child[idx+1] = b;
            }
        }

        if (idx != 0 && mem_child[idx-1] != nullptr && mem_child[idx-1]->n >= t) {
            borrowFromPrev(idx, disk);
        } else if (idx != n && mem_child[idx+1] != nullptr && mem_child[idx+1]->n >= t) {
            borrowFromNext(idx, disk);
        } else {
            if (idx != n) {
                merge(idx, disk);
            } else {
                merge(idx-1, disk);
            }
        }
    }
    void borrowFromPrev(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        BTreeNode<M, K,O>* child = mem_child[idx];
        BTreeNode<M, K,O>* sibling = mem_child[idx-1];
        
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
        disk.writeNode(child->diskidx,*child);
        disk.writeNode(sibling->diskidx,*sibling);
        disk.writeNode(this->diskidx,*this);
    }
    void borrowFromNext(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        BTreeNode<M, K,O>* child = mem_child[idx];
        BTreeNode<M, K,O>* sibling = mem_child[idx+1];
        
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
        disk.writeNode(child->diskidx,*child);
        disk.writeNode(sibling->diskidx,*sibling);
        disk.writeNode(this->diskidx,*this);
    }
    void merge(int idx, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
        BTreeNode<M, K,O>* child = mem_child[idx];
        BTreeNode<M, K,O>* sibling = mem_child[idx+1];
        
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
        disk.writeNode(child->diskidx,*child);
        disk.writeNode(this->diskidx,*this);
    }
    
public:

    // Constructor
    BTreeNode(bool leaf = true, int d = 0) 
        : t(ceil(M/2) - 1), isLeaf(leaf), n(0), diskidx(d) {}
    
    // Destructor
    ~BTreeNode() {
        for (auto& child : mem_child) {
            if (child != nullptr) {
                delete child;
            }
        }
    }
    // Remove a key from the subtree rooted with this node
    void remove(uint32_t k, DiskManager<BTreeNode<M, K,O>,K,O>& disk) {
        int idx = findKey(k);

        // The key to be removed is present in this node
        if (idx < n && keys[idx] == k) {
            // If the node is a leaf node
            if (isLeaf) {
                removeFromLeaf(idx, disk);
            } else {
                removeFromNonLeaf(idx, disk);
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
                BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx]));
                mem_child[idx] = b;
            }

            // If the child where the key is supposed to exist has less than t keys,
            // fill that child
            if (mem_child[idx] != nullptr && mem_child[idx]->n < t) {
                fill(idx, disk);
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
                    BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx-1]));
                    mem_child[idx-1] = b;
                }
                if (mem_child[idx-1] != nullptr) {
                    mem_child[idx-1]->remove(k, disk);
                }
            } else {
                // Load child[idx] if needed (might have changed after fill)
                if (mem_child[idx] == nullptr && file_child[idx] != 0) {
                    BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[idx]));
                    mem_child[idx] = b;
                }
                if (mem_child[idx] != nullptr) {
                    mem_child[idx]->remove(k, disk);
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
    void setMemChild(int index, BTreeNode<M, K,O>* child) { 
        if (index >= 0 && index < 2*t) mem_child[index] = child; 
    }
    void setKeyCount(int count) { n = count; }
    void setIsLeaf(bool leaf) { isLeaf = leaf; }
    
    // Public methods
    BTreeNode<M, K,O>* search(uint32_t k, DiskManager<BTreeNode<M, K,O>,K,O>& disk){
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
            BTreeNode<M, K,O>* b = new BTreeNode<M, K,O>(disk.readNode(file_child[i]));
            mem_child[i] = b;
        }

        // Search in child
        return (mem_child[i] != nullptr) ? mem_child[i]->search(k, disk) : nullptr;
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
};
template <int M,int K, int O>
class BTree {
private:
    BTreeNode<M, K,O>* root;
    DiskManager<BTreeNode<M, K,O>, K, O> disk;
    
    // Recursive delete subtree
    void deleteSubtree(BTreeNode<M, K,O>* node) {
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
    BTreeNode<M, K,O>* loadNode(uint32_t offset) {
        // Check if already loaded (simple implementation)
        // In advanced version, use LRU cache
        BTreeNode<M, K,O>* u = new BTreeNode<M, K,O>(disk.readNode(offset));
        return u;
    }
    
    // Save a node to disk
    void saveNode(BTreeNode<M, K,O>* node) {
        if (node == nullptr) return;
        disk.writeNode(node->diskidx, *node);
    }
    // Constructor
    BTree(const string& filename, int _t = 4, const int MAX = MAX_USERS) 
        : disk(filename), root(nullptr) {
        
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
        
        BTreeNode<M, K,O>* result = root->search(k, this->disk);
        return (result != nullptr);
    }
    
    // Insert a key
    void insert(uint32_t k) {
        // If tree is empty
        if (root == nullptr) {
            root = new BTreeNode<M, K,O>(true);
            root->keys[0] = k;
            root->n = 1;
            root->diskidx = disk.allocateBlock();
            saveNode(root);
        } 
        else {
            // If root is full, tree grows in height
            if (root->n == M) {
                BTreeNode<M, K,O>* newRoot = new BTreeNode<M, K,O>(false, disk.allocateBlock());
                newRoot->setMemChild(0, root);
                newRoot->splitChild(0, root, this->disk);
                
                // Decide which child will have new key
                int i = 0;
                if (newRoot->keys[0] < k) {
                    i++;
                }
                newRoot->mem_child[i]->insertNonFull(k, this->disk);
                
                root = newRoot;
                saveNode(root);
            } else {
                root->insertNonFull(k, this->disk);
            }
        }
    }
    
    // Remove a key
    // Remove a key from the B-Tree
    void remove(uint32_t k) {
        if (root == nullptr) {
            cout << "Tree is empty\n";
            return;
        }

        // Call remove for root
        root->remove(k, this->disk);

        // If root becomes empty after deletion
        if (root->n == 0) {
            BTreeNode<M, K,O>* temp = root;

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
        uint32_t idx = disk.getFirstUsedBlock();
        if(idx == -1) return;
        root = loadNode(idx);
        if (root != nullptr) {
            // Load children recursively
            loadChildren(root);
        }
    }
    
private:
    // Recursive save
    void saveSubtree(BTreeNode<M, K,O>* node) {
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
    void loadChildren(BTreeNode<M, K,O>* node) {
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
#endif