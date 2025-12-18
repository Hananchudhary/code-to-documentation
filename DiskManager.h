#ifndef DISKMANAGER_H

#define DISKMANAGER_H
#include<iostream>
#include<vector>
#include<fstream>
#include<cstdint>
#include"node_types.h"
using namespace std;
template <typename T>
class DiskManager {
private:
    fstream file;
    char* bitmap;
    int M;
    void persistBitmap() {
        file.seekp(0, ios::beg);
        file.write(bitmap, sizeof(bitmap));
        file.flush();
    }
public:
    DiskManager(string filename, const int MAX): bitmap{new char[MAX/8]}, M{MAX} {
        file.open(filename.c_str(), ios::in | ios::out | ios::binary);
        if (!file) {
            file.open(filename, ios::out | ios::binary);
            for (int i = 0; i < M / 8; i++) bitmap[i] = 0;
            file.write((char*)bitmap, sizeof(bitmap));
            file.close();
            file.open(filename, ios::in | ios::out | ios::binary);
        }
        file.seekg(0, ios::beg);
        file.read((char*)bitmap, sizeof(bitmap));
        cout << "";
    }

    ~DiskManager() {
        delete bitmap;
        file.seekp(0, ios::beg);
        file.write((char*)bitmap, sizeof(bitmap));
        file.close();
    }

    int allocateBlock() {
        for (int i = 0; i < M/8; i++) {

            unsigned char freeMask = ~bitmap[i]; // 1 = free, 0 = used

            if (freeMask != 0x00) {

                // scan bits from LSB → MSB
                for (int bit = 0; bit < 8; bit++) {

                    if (freeMask & (1 << bit)) {  // free bit found

                        int idx = i * 8 + bit;

                        // mark bit used
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
                        int b = 7-bit;
                        return i * 8 + b;
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
    T readNode(int index) {
        T node;
        file.seekg(sizeof(T) * index + (sizeof(bitmap)*(M/8)));
        node.readFromFile(sizeof(T) * index + (sizeof(bitmap)*(M/8)), file);
        // file.read(reinterpret_cast<char*>(&node), sizeof(T));
        return node;
    }

    void writeNode(int index, T& node) {
        file.seekp(sizeof(T) * index + (sizeof(bitmap)*(M/8)));
        node.writeToFile(sizeof(T) * index + (sizeof(bitmap)*(M/8)), file);
        // file.write(reinterpret_cast<const char*>(&node), sizeof(T));
    }
};
#endif