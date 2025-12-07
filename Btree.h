#include <iostream>
using namespace std;

template <typename T>
class BTreeNode {
public:
T* keys; // array of keys
BTreeNode<T>** child; // array of child pointers
int t; // minimum degree
int n; // current number of keys
bool leaf;


BTreeNode(int _t, bool _leaf = true) {
    t = _t;
    leaf = _leaf;
    keys = new T[2 * t - 1]{};
    child = new BTreeNode<T>*[2 * t]{};
    n = 0;
}


void traverse(){
    int i = 0;
    while(i < this->n){
        if(!this->leaf){
            this->child[i]->traverse();
        }
        cout << this->keys[i++] << " ";
    }
    if(!this->leaf)
        this->child[i]->traverse();
}
BTreeNode<T>* search(const T& k){
    if(this->n == 0) return nullptr;
    if(this->keys[0] > k){
        if(!this->child[0]) return nullptr;
        return this->child[0]->search(k);
    }
    if(k > this->keys[this->n - 1]){
        if(!this->child[this->n]) return nullptr;
        return this->child[this->n]->search(k);
    } 
    for(int i = 0;i<this->n - 1;i++){
        if(this->keys[i] == k) return this;
        if(k > this->keys[i] && k < this->keys[i]){
            if(!this->child[i+1]) return nullptr;
            return this->child[i+1]->search(k);
        } 
    }
    return nullptr;
}


void insertNonFull(const T& k){
    int idx = this->findKey(k);
    if(this->leaf){
        for(int i = this->n;i > idx;i--){
            this->keys[i] = this->keys[i-1];
            this->child[i] = this->child[i-1];
        }
        this->keys[idx] = k;
    }
    else if(this->child[idx]) {
        if(this->child[idx]->n == (2*t - 1)){
            this->splitChild(idx, this);
        }
        if(k > this->child[idx]) 
            idx++;
        this->child[idx]->insertNonFull(k);
    }
}
void splitChild(int idx, BTreeNode<T>* y){
    for(int i = (2*t - 1);i > idx;i--){
        y->child[i] = y->child[i-1];
    }
    if(y->child[idx+1]) delete y->child[idx+1];
    y->child[idx+1] = new BTreeNode<T>(y->t, false);
    y->child[idx+1]->leaf = y->child[idx]->leaf;
    while(y->child[idx]->n > y->t){
        int i = y->child[idx+1]->findKey(y->keys[idx]);
        y->child[idx+1]->keys[i] = y->keys[idx];
        y->keys[idx] = y->child[idx]->keys[y->child[idx]->n-1];
        y->child[idx]->keys[y->child[idx]->n-1] = 0;
        y->child[idx+1]->child[i] = y->child[idx]->child[y->child[idx]->n-1];
        y->chilf[idx]->n--;
        y->child->[idx+1]->n++;
    }
}


int findKey(const T& k){
    if(this->n == 0 || k < this->keys[this->n] || k > this->keys[this->n-1])
        return this->n;
    for(int i = 0;i<this->n - 1;i++){
        if(k > this->keys[i] && k < this->keys[i+1])
            return i;
    }
    return -1;
}
void remove(const T& k){
    int idx = this->findKey(k);
    int i = this->child[idx]->findKey(k);
    if(this->child[idx]->keys[i] == k){
        if(this->child[idx]->leaf){
            if(this->child[idx]->n <= t){
                this->fill(idx);
                i = this->child[idx]->findKey(k);
            }
            this->child[idx]->removeFromLeaf(i);
        }
        else{
            this->child[idx]->removeFromNonLeaf(i);
        }
    }
    else
        this->child[idx]->remove(k);
}
void removeFromLeaf(int idx){
    for(int i = idx;i<this->n - 1;i++){
        this->keys[i] = this->keys[i+1];
    }
    this->keys[--this->n] = 0;
}
void removeFromNonLeaf(int idx){
    int prv = getPred(idx);
    this->keys[idx] = prv;
    remove(prv);
}
int getPred(int idx){
    BTreeNode<T>* prv = this;   
    BTreeNode<T>* ct = this->child[idx];
    while(ct){
        prv = ct;
        ct = ct->child[ct->n];
    }
    return prv->keys[prv->n-1];
}
int getSucc(int idx){
    BTreeNode<T>* prv = this;   
    BTreeNode<T>* ct = this->child[idx+1];
    while(ct){
        prv = ct;
        ct = ct->child[0];
    }
    return prv->keys[0];
}
void fill(int idx){
    while(this->child[idx-1] && this->child[idx-1]->n > this->t && this->child[idx]->n < t){
        this->borrowFromPrev(idx);
    }
    while(this->child[idx+1] && this->child[idx+1]->n > this->t && this->child[idx]->n < t){
        this->borrowFromNext(idx);
    }
    if(this->child[idx]->n < t){
        merge(idx);
    }
}
void borrowFromPrev(int idx){
    BTreeNode<T>* x = this->child[idx-1];
    BTreeNode<T>* y = this->child[idx];
    for(int i = y->n;i>0;i--){
        y->keys[i] = y->keys[i-1];
        y->child[i] = y->child[i-1];
    }
    y->keys[0] = this->keys[idx];
    y->n++;
    this->keys[idx] = x->keys[x->n - 1];
    y->child[0] = x->child[x->n];
    x->child[x->n] = nullptr;
    x->n--;
}
void borrowFromNext(int idx){
    BTreeNode<T>* x = this->child[idx];
    BTreeNode<T>* y = this->child[idx+1];
    x->keys[x->n -1] = this->keys[idx];
    this->keys[idx] = y->keys[0];
    x->child[++x->n] = y->child[0];
    for(int i = 0;i<y->n;i++){
        y->keys[i] = y->keys[i+1];
        y->child[i] = y->child[i+1];
    }
    y->child[y->n] = nullptr;
    y->n--;
}
void merge(int idx){
    BTreeNode<T>* temp = new BTreeNode<T>(this->t);
    temp->leaf = this->child[idx]->leaf;
    int j = 0;
    for(int i = 0;i<this->child[idx]->n;i++){
        temp->keys[j] = this->child[idx]->keys[i];
        temp->child[j++] = this->child[idx]->child[i];
        this->n++;
    }
    temp->keys[j++] = this->keys[idx];
    for(int i = 0;i<this->child[idx+1]->n;i++){
        temp->keys[j] = this->child[idx+1]->keys[i];
        temp->child[j++] = this->child[idx+1]->child[i];
        this->n++;
    }
    delete this->child[idx+1];
    delete this->child[idx];
    this->child[idx] = temp;
    for(int i = this->n-1;i > idx;i--){
        this->keys[i - 1] = this->keys[i];
        this->child[i] = this->child[i+1];
    }
    this->n--;
}
};


// =====================================================
// B-Tree Class
// =====================================================
template <typename T>
class BTree {
private:
BTreeNode<T>* root;
int t; // minimum degree


public:
BTree(int _t) {
    root = nullptr;
    t = _t;
}


void traverse() {
    if (root != nullptr) root->traverse();
        cout << "\n";
}


BTreeNode<T>* search(const T& k) {
    return (root == nullptr) ? nullptr : root->search(k);
}


void insert(const T& k){
    if(!root){
        root = new BTreeNode<T>(t);
        root->insertNonFull(k);
        return;
    }
    if(root->n == (2*t - 1)){
        BTreeNode<T>* s = new BTreeNode<T>(t, false);
        s->child[0] = root;
        s->splitChild(0, s);
        root = s;
        int idx = 0;
        if(k > root->keys[0]) idx++;
        root->child[idx]->insertNonFull(k);
    }
    else{
        root->insertNonFull(k);
    }
}
void remove(const T& k){
    if(!root)
        return;
    int idx = root->findKey(k);
    if(root->keys[idx] == k){
        if(root->leaf){
            root->removeFromLeaf(idx);
        }
        else{
            root->removeFromNonLeaf(idx);
        }
    }
    root->remove(k);
    if(root->n == 0){
        BTreeNode<T>* tmp = root;
        if(root->leaf){
            delete root;
            root = nullptr;
        }
        else{
            root = root->child[0];
            delete temp;
        }
    }
}
};