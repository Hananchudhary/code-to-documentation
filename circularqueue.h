#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
using namespace std;

template<typename T>
class CircularQueue {
    size_t st;
    size_t end;
    size_t static_array_size;
    bool isfull;
    T* arr;
    mutex mtx;
    condition_variable cv_not_empty;
    condition_variable cv_not_full;

public:
    CircularQueue() : st{0}, end{0}, static_array_size{50}, isfull{false}, arr{new T[static_array_size]} {}
    CircularQueue(size_t cap) : st{0}, end{0}, static_array_size{cap}, isfull{false}, arr{new T[cap]} {}

    // Blocking enqueue
    void enqueue(const T& value) {
        unique_lock<mutex> lock(mtx);
        cv_not_full.wait(lock, [this]{ return !isfull; });

        arr[end] = value;
        end = (end + 1) % static_array_size;
        if (end == st) isfull = true;

        cv_not_empty.notify_one(); // notify exactly one waiting thread
    }

    // Non-blocking enqueue
    bool try_enqueue(const T& value) {
        unique_lock<mutex> lock(mtx);
        if (isfull) return false;

        arr[end] = value;
        end = (end + 1) % static_array_size;
        if (end == st) isfull = true;

        cv_not_empty.notify_one();
        return true;
    }

    // Blocking dequeue
    T dequeue() {
        unique_lock<mutex> lock(mtx);
        cv_not_empty.wait(lock, [this]{ return !(st == end && !isfull); });

        T temp = arr[st];
        st = (st + 1) % static_array_size;
        if (isfull) isfull = false;

        cv_not_full.notify_one(); // notify one thread waiting to enqueue
        return temp;
    }

    // Non-blocking dequeue
    bool try_dequeue(T& out) {
        unique_lock<mutex> lock(mtx);
        if (st == end && !isfull) return false;

        out = arr[st];
        st = (st + 1) % static_array_size;
        if (isfull) isfull = false;

        cv_not_full.notify_one();
        return true;
    }

    size_t size() const {
        unique_lock<mutex> lock(mtx);
        if (isfull) return static_array_size;
        return ((static_array_size + end) - st) % static_array_size;
    }

    bool empty() const {
        unique_lock<mutex> lock(mtx);
        return (end == st && !isfull);
    }

    size_t capacity() const { return static_array_size; }

    ~CircularQueue() { delete[] arr; }
};
