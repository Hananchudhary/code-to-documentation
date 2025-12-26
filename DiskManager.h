#ifndef DISKMANAGER_H
#define DISKMANAGER_H
#include<iostream>
#include<vector>
#include<fstream>
#include<cstdint>
#include"node_types.h"
using namespace std;
template <typename T, int M, const int O>
class DiskManager {
private:
    fstream file;
    string filenameStr;
    char bitmap[M/8]{};
    void persistBitmap() {
        file.seekp(O, ios::beg);
        file.write(bitmap, sizeof(bitmap));
        file.flush();
    }
public:
    DiskManager(string filename){
        strcpy(&filenameStr[0], filename.c_str());
        file.open(filename.c_str(), ios::in | ios::out | ios::binary);
        if (!file) {
            file.open(filename, ios::out | ios::binary);
            for (int i = 0; i < M / 8; i++) bitmap[i] = 0;
            file.write((char*)bitmap, sizeof(bitmap));
            file.close();
            file.open(filename, ios::in | ios::out | ios::binary);
        }
        file.seekg(O, ios::beg);
        file.read((char*)bitmap, sizeof(bitmap));
    }
    DiskManager(const DiskManager& other) {
        // Copy bitmap
        strcpy(&filenameStr[0], other.fileName().c_str());
        for (int i = 0; i < M / 8; ++i)
            bitmap[i] = other.bitmap[i];

        file.open(other.fileName(), ios::in | ios::out | ios::binary);
        if (!file) {
            throw runtime_error("Failed to open file in copy constructor");
        }
    }
    ~DiskManager() {
        file.seekp(O, ios::beg);
        file.write((char*)bitmap, sizeof(bitmap));
        file.close();
    }
    string fileName() const {
        return filenameStr; // you need to store it
    }
    int allocateBlock() {
        for (int i = 0; i < M/8; i++) {
            unsigned char freeMask = ~bitmap[i]; // 1 = free, 0 = used
            if (freeMask != 0x00) {
                for (int bit = 0; bit < 8; bit++) {
                    if (freeMask & (1 << bit)) {
                        int idx = i * 8 + bit;
                        bitmap[i] |= (1 << bit);
                        return idx;
                    }
                }
            }
        }

        return -1; // no free blocks
    }

    uint32_t getFirstUsedBlock() {
        for (uint32_t i = 0; i < M / 8; i++) {
            if (bitmap[i] != 0) {  // This byte has at least one used block
                for (int bit = 0; bit < 8; bit++) {
                    if (bitmap[i] & (1 << bit)) {
                        return i * 8 + bit;
                    }
                }
            }
        }
        return -1; // No used block found
    }
    void freeBlock(int idx){
        unsigned char k = 1 << (idx%8);
        k = ~k;
        bitmap[idx/8] &= k;
    }
    void setBlock(int idx) {
        int byte = idx / 8;        // which byte
        int bit  = idx % 8;        // which bit inside that byte
        bitmap[byte] |= (1 << bit);
    }
    T readNode(int index) {
        T node;
        file.seekg(sizeof(T) * index + (sizeof(bitmap))+O);
        file.read(reinterpret_cast<char*>(&node), sizeof(T));
        return node;
    }

    void writeNode(int index, T& node) {
        file.seekp(sizeof(T) * index + (sizeof(bitmap))+O);
        file.write(reinterpret_cast<const char*>(&node), sizeof(T));
    }
};
#endif