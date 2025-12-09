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
    }

    ~DiskManager() {
        delete bitmap;
        file.seekp(0, ios::beg);
        file.write((char*)bitmap, sizeof(bitmap));
        file.close();
    }

    int allocateBlock() {
        for(int i = 0;i<M/8;i++){
            unsigned char k = ~bitmap[i];
            if(k != 0x00){
                unsigned char it = 0x01;
                while(it != 0x00){
                    unsigned char ch = k & it;
                    if(ch != 0x00){
                        it = ch;
                        break;
                    }
                    it <<= 1;
                }
                int bit = 0;
                unsigned char temp = it;
                while (temp >>= 1) bit++;
                int idx = i * 8 + bit;
                bitmap[idx/8] |= (1 << (idx%8));
                return idx;
            }
        }
        return -1; 
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
    T readNode(int index) {
        T node;
        file.seekg(sizeof(T) * index + (sizeof(bitmap)*(M/8)));
        node.readFromFile(sizeof(T) * index + (sizeof(bitmap)*(M/8) + 1), file);
        // file.read(reinterpret_cast<char*>(&node), sizeof(T));
        return node;
    }

    void writeNode(int index, T& node) {
        file.seekp(sizeof(T) * index + (sizeof(bitmap)*(M/8)));
        node.writeToFile(sizeof(T) * index + (sizeof(bitmap)*(M/8) + 1), file);
        // file.write(reinterpret_cast<const char*>(&node), sizeof(T));
    }
};
#endif