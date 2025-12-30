#pragma once
#include<iostream>
#include"Btree.h"
#include"AVL.h"
using namespace std;
uint32_t myHash(const string& pass, const string& username){
    int h = 1;
    int s = pass.size();
    int i = 0;
    while(i<pass.size() && i < username.size()){
        h = h*(pass[i] + username[i] - 31) % MAX_FILES;
        i++;
    }
    while(i<pass.size()){
        h = h *(pass[i] - 31) % MAX_FILES;
        i++;
    }
    while(i<username.size()){
        h = h *(username[i] - 31) % MAX_FILES;
        i++;
    }
    return h;
}
const int offset1 = ((MAX_FILES) * sizeof(BTreeNode<5,MAX_FILES, 0>)) + (MAX_FILES/8);
const int offset2 = offset1 + MAX_FILES * sizeof(AVLNode<FileEntry>) + MAX_FILES/8;
class FileDBManager{
    BTree<5, MAX_FILES, 0> files_idx;
    vector<AVLTree<FileEntry>> files;
    DiskManager<AVLNode<User>, MAX_FILES, offset1> dm;
    DiskManager<BlockHdr, MAX_BLOCKS, offset2> dataDM;
    fstream file;
    int size;
    public:
    FileDBManager(): files_idx(FilesDB_FILE, 4, MAX_FILES), size{0}, dm(FilesDB_FILE), dataDM(FilesDB_FILE){
        files.resize(MAX_FILES);
        file.open(FilesDB_FILE, std::ios::binary | std::ios::in | std::ios::out);
    }
    int writeDataToFile(int startBlock, const string& data) {
        int remaining = data.size();
        uint32_t written = 0;
        int curr = startBlock;
        int nxt = 0;
        while (remaining > 0) {
            BlockHdr hdr{};
            file.seekg(curr, ios::beg);
            file.read(reinterpret_cast<char*>(&hdr), sizeof(BlockHdr));
            nxt = hdr.nxt;
            hdr.isValid = 1;
            uint32_t toWrite = min(
                remaining,
                BLOCK_SIZE
            );
            hdr.size = toWrite;
            if(remaining > BLOCK_SIZE && nxt <= 0){
                nxt = dataDM.allocateBlock();
                if (nxt <= 0)
                    return static_cast<int>(OFSErrorCodes::ERROR_NO_SPACE);
                nxt = nxt * (sizeof(BlockHdr) + BLOCK_SIZE)
                       + offset2 + MAX_BLOCKS / 8;
            }
            hdr.nxt = nxt;
            file.seekp(curr, ios::beg);
            file.write(reinterpret_cast<const char*>(&hdr), sizeof(BlockHdr));
            file.write(data.data() + written, toWrite);
            written += toWrite;
            remaining -= toWrite;
            curr = nxt; 
        }

        while(curr > 0){
            BlockHdr hdr;
            file.seekg(curr, ios::beg);
            file.read(reinterpret_cast<char*>(&hdr), sizeof(BlockHdr));
            nxt = hdr.nxt;
            hdr.isValid = 0;
            hdr.size = 0;
            hdr.nxt = 0;
            file.seekp(curr, ios::beg);
            file.write(reinterpret_cast<const char*>(&hdr), sizeof(BlockHdr));
            curr = nxt;
        }
        return 0;
    }
    string readDataFromFile(int startBlock) {
        string result;
        int curr = startBlock;

        while (curr > 0) {
            BlockHdr hdr;
            file.seekg(curr, ios::beg);
            file.read(reinterpret_cast<char*>(&hdr), sizeof(BlockHdr));

            if (!hdr.isValid || hdr.size == 0)
                break;

            vector<char> buffer(hdr.size);
            file.seekg(curr+sizeof(BlockHdr), ios::beg);
            file.read(buffer.data(), hdr.size);
            result.append(buffer.data(), hdr.size);

            curr = hdr.nxt;
        }
        return result;
    }

    int createFile(const string& username,const string& filename, const string data){
        uint32_t idx = myHash(username, filename);
        int i = idx;
        if(size>MAX_USERS) return static_cast<int>(OFSErrorCodes::ERROR_MAX_USERS);
        if(!files[idx].search(username)){
            if(!files_idx.search(idx)){
                files_idx.insert(idx);
                dm.setBlock(idx);
            }
            else{
                i = dm.allocateBlock();
            } 
            int b = dataDM.allocateBlock();
            FileEntry f(username, filename, b*(sizeof(BlockHdr)+BLOCK_SIZE ) + offset2+MAX_BLOCKS/8);

            files[idx].insert(username+filename, f,file, (i*sizeof(AVLNode<FileEntry>)+offset1+MAX_FILES/8));
            this->writeDataToFile(f.inode, data);
            size++;
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
        return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
    }
    int ReadFile(const string& username,const string& filename, string& data) {
        uint32_t idx = myHash(username, filename);
        if (!files_idx.search(idx)) return -7;
        AVLNode<FileEntry>* us = files[idx].search(username+filename);
        if(!us){
            files[idx].loadTree(file,(MAX_FILES/8 + offset1 + idx*sizeof(AVLNode<FileEntry>)));
            us = files[idx].search(username+filename);
        }
        if(!us) return -7;
        data = readDataFromFile(us->value.inode);
        return 0;
    }
    int editFile(const string username, const string filename, const string data){
        uint32_t idx = myHash(username, filename);
        int i = idx;
        AVLNode<FileEntry>* fs = files[idx].search(username+filename);
        if(fs){
            this->writeDataToFile(fs->value.inode, data);
            return 0;
        }
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }
    vector<string> getFilesofUser(const string username){
        vector<string> res;

        for (int i = 0; i < MAX_FILES; i++) {
            vector<string> temp = files[i].getFiles(username);
            res.insert(res.end(), temp.begin(), temp.end());
        } 
        return res;
    }
    bool hasFileAccess(const string username, const string filename){
        int idx = myHash(username, filename);
        AVLNode<FileEntry>* f = files[idx].search(username+filename);
        if(!f) return false;
        if(strcmp(&username[0],f->value.userName) != 0 || strcmp(&filename[0], f->value.name) != 0){
            return false;
        }
        return true;
    }
    bool getUser(const string filename, FileEntry& fe){
        for (int i = 0; i < MAX_FILES; i++) {
            AVLNode<FileEntry>* fs = files[i].search(filename);
            if(fs){
                fe = fs->value;
                return true;
            }
        }
        return false;
    }
};