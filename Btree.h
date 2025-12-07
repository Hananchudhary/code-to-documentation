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
    if(!this->leaf && this->child[i])
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
        this->n++;
    }
    else if(this->child[idx]) {
        if(this->child[idx]->n == (2*t - 1)){
            this->splitChild(idx);
            if(k > this->keys[idx]) 
                idx++;
        }
        this->child[idx]->insertNonFull(k);
    }
}
void splitChild(int idx){
    BTreeNode<T>* x = this->child[idx];
    BTreeNode<T>* z = new BTreeNode<T>(t);
    z->leaf = x->leaf;
    for(int i = this->n;i>idx;i--){
        this->keys[i] = this->keys[i-1];
        this->child[i] = this->child[i-1];
    }
    for(int i = 0,j = this->t+1;j < x->n;j++, i++){
        z->keys[i] = x->keys[j];
        z->child[i] = x->child[j+1];
        x->child[j] = nullptr;
        x->keys[j] = 0;
        z->n++;
    }
    x->n = x->n - z->n;
    this->keys[idx] = x->keys[this->t];
    x->keys[this->t] = 0;
    x->n--;
    this->child[idx+1] = z;
    this->n++;
}


int findKey(const T& k){
    if(this->n == 0 || k < this->keys[0])
        return 0;
    if(k > this->keys[this->n-1])
        return this->n;
    if(k == this->keys[this->n - 1])
        return this->n-1;
    for(int i = 0;i<this->n - 1;i++){
        if(k == this->keys[i])
            return i;
        if(k > this->keys[i] && k < this->keys[i+1])
            return i+1;
    }
    return -1;
}
void remove(const T& k){
    int idx = this->findKey(k);
    if(!this->child[idx])
        return;
    int i = this->child[idx]->findKey(k);
    if(i != -1 && this->child[idx]->keys[i] == k){
        if(this->child[idx] && this->child[idx]->leaf){
            if(this->child[idx]->n <= t){
                this->fill(idx);
                i = this->child[idx]->findKey(k);
            }
            this->child[idx]->removeFromLeaf(i);
        }
        else if(this->child[idx]){
            this->child[idx]->removeFromNonLeaf(i);
        }
    }
    else
        this->child[idx]->remove(k);
}
void removeFromLeaf(int idx){
    for(int i = idx;i<this->n-1;i++){
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
    while(idx > 0 && this->child[idx-1] && this->child[idx-1]->n > this->t && this->child[idx]->n < t){
        this->borrowFromPrev(idx);
    }
    while(idx < (2*t) && this->child[idx+1] && this->child[idx+1]->n > this->t && this->child[idx]->n < t){
        this->borrowFromNext(idx);
    }
    if(this->child[idx]->n <= t){
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
        temp->n++;
    }
    temp->keys[j++] = this->keys[idx];
    temp->n++;
    for(int i = 0;i<this->child[idx+1]->n-1;i++){
        temp->keys[j] = this->child[idx+1]->keys[i];
        temp->child[j++] = this->child[idx+1]->child[i];
        temp->n++;
    }
    this->keys[idx] = this->child[idx+1]->keys[this->child[idx+1]->n-1];
    delete this->child[idx+1];
    this->child[idx+1] = nullptr;
    delete this->child[idx];
    this->child[idx] = temp;
    for(int i = this->n-1;i > idx;i--){
        this->keys[i - 1] = this->keys[i];
        this->child[i] = this->child[i+1];
    }
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
        root->keys[0] = k;
        root->n++;
        return;
    }
    if(root->n == (2*t)){
        BTreeNode<T>* s = new BTreeNode<T>(t, false);
        s->child[0] = root;
        s->splitChild(0);
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
    else{
        root->remove(k);
    }
    if(root->n == 0){
        BTreeNode<T>* tmp = root;
        if(root->leaf){
            delete root;
            root = nullptr;
        }
        else{
            root = root->child[0];
            delete tmp;
        }
    }
}
};