#pragma once
#include <iostream>
#include <stdexcept>
using namespace std;
template <typename T>
class Stack {
private:
    int size;
    int capacity;
    T* arr;

public:
    // Constructors
    Stack() : size(0), capacity(10), arr(new T[capacity]) {}
    Stack(int cap) : size(0), capacity(cap), arr(new T[capacity]) {}

    // Destructor
    ~Stack() {
        delete[] arr;
    }

    // Size
    int get_size() const {
        return size;
    }

    // Empty check
    bool empty() const {
        return size == 0;
    }

    // Top element
    T top() const {
        if (empty())
            throw std::runtime_error("Stack is empty");
        return arr[size - 1];
    }

    // Push
    void push(const T& data) {
        if (size == capacity)
            throw std::runtime_error("Stack is full");
        arr[size++] = data;
    }

    // Pop
    void pop() {
        if (empty())
            throw std::runtime_error("Stack is empty");
        --size;
    }

    // Print (top to bottom)
    void print() const {
        for (int i = size - 1; i >= 0; --i)
            std::cout << arr[i] << " ";
        std::cout << std::endl;
    }
};
