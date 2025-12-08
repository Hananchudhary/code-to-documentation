#pragma once
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
    char bitmap[MAX_BLOCKS / 8]; // 1 bit per block
    void persistBitmap() {
        file.seekp(0, ios::beg);
        file.write(bitmap, sizeof(bitmap));
        file.flush();
    }
public:
    DiskManager(string filename) {
        file.open(filename.c_str(), ios::in | ios::out | ios::binary);
        if (!file) {
            file.open(DB_FILE, ios::out | ios::binary);
            for (int i = 0; i < MAX_BLOCKS / 8; i++) bitmap[i] = 0;
            file.write((char*)bitmap, sizeof(bitmap));
            file.close();
            file.open(DB_FILE, ios::in | ios::out | ios::binary);
        }
        file.seekg(0, ios::beg);
        file.read((char*)bitmap, sizeof(bitmap));
    }

    ~DiskManager() {
        file.seekp(0, ios::beg);
        file.write((char*)bitmap, sizeof(bitmap));
        file.close();
    }

    int allocateBlock() {
        for(int i = 0;i<MAX_BLOCKS/8;i++){
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
    void freeBlock(int idx){
        unsigned char k = 1 << (idx%8);
        k = ~k;
        bitmap[idx/8] &= k;
    }
    T readNode(int index) {
        T node;
        file.seekg(sizeof(T) * index);
        file.read(reinterpret_cast<char*>(&node), sizeof(T));
        return node;
    }

    void writeNode(int index, T& node) {
        const int BITMAP_SIZE = MAX_BLOCKS / 8;

        file.seekg(BITMAP_SIZE + sizeof(T) * index);
        file.seekp(BITMAP_SIZE + sizeof(T) * index);
    }
};