#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include <stdexcept>
#include<iostream>
using namespace std;
template <class T>
class maxHeap {
private:

    T* arr;
    int size;
    int capacity;

    // --- Private Helper Functions ---
    int getLeftChild(int parent){
        return (2*parent + 1);
    }
    int getRightChild(int parent){
        return (2*parent + 2);
    }
    int getParent(int child){
        return (child - 1)/2;
    }
    void heapifyUp(int node){
        int par = getParent(node);
        while(par>=0 && arr[par]<arr[node]){
            swap(arr[par], arr[node]);
            node = par;
            par = getParent(node);
        }
    }
    void heapifyDown(int node){
        int left = getLeftChild(node);
        int right = getRightChild(node);
        int i = node, idx = 0;
        while(left<size || right<size){
            T val;
            bool flag = false;
            if(left<size && arr[left]>=arr[right] && arr[i] < arr[left]){
                val = arr[left];
                idx = left;
                flag = true;
            }
            else if(right<size && arr[left]<arr[right] && arr[i] < arr[right]){
                val = arr[right];
                idx = right;
                flag = true;
            } 
            if(!flag) break; 
            swap(arr[i], arr[idx]);
            i = idx;
            left = getLeftChild(i);
            right = getRightChild(i);
        }
    }
    void double_capacity(){
        this->capacity=this->capacity*2;
        T* temp = new T[this->capacity];
        for(int i=0;i<this->size;i++) temp[i] = this->arr[i];
        delete[] this->arr;
        this->arr = temp; 
    }

public:

    // --- Public Member Functions ---
    maxHeap():arr{new T[50]}, size{0}, capacity{50}{}
    void heapify(T* _arr, int _capacity){
        T* a = this->arr;
        this->arr = _arr;
        int s = this->size;
        this->size = _capacity;
        for(int i=0;i<_capacity;i++){
            heapifyDown(i);
        }
        _arr = this->arr;
        this->arr = a;
        this->size = s;
    }
    maxHeap(T* _arr, int _capacity){
        this->arr = new T[_capacity*2];
        this->size = _capacity;
        this->capacity = _capacity*2;
        for(int i=0;i<_capacity;i++) arr[i]=_arr[i];
        heapify(this->arr, this->size);
    }
    T getMax(){
        if(!empty()) return this->arr[0];
        return T{};
    }
    T extractMax(){
        if(!empty()){
            T num = this->arr[0];
            this->erase(this->arr[0]);
            return num;
        }
        return T{};
    }
    void insert(T _key){
        if(this->size +1 > this->capacity) double_capacity();
        this->arr[this->size] = _key;
        heapifyUp(this->size++);
    }
    void erase(T _key){
        int i =0;
        while(i<this->size){
            if(this->arr[i]==_key){
                heapifyDown(i);
                break;
            }
            i++;
        }
        while(i<this->size){
            if(this->arr[i] == _key) break;
            i++;
        }
        while(i<this->size - 1){
            this->arr[i] = this->arr[i+1];
            i++;
        } 
        this->size--;
        this->heapify(this->arr, this->size);
    }
    bool empty(){return size==0;}
    int get_size(){return size;}
    static void sort(T* _arr, int _capacity){
        maxHeap q(_arr, _capacity);
        int i =_capacity-1;
        while(i>-1) 
            _arr[i--] = q.extractMax();
    }

    ~maxHeap(){
        delete[] arr;
    }
};

#endif // MAX_HEAP_H