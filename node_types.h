#ifndef NODE_TYPES_H
#define NODE_TYPES_H
#include<iostream>
#include<cstdint>
#include<memory.h>
#include"config.h"
using namespace std;
struct User {        // Fixed size for B-tree
    char userName[MAX_USERNAME_LEN+1];
    char password[MAX_USERNAME_LEN+1];
    uint8_t is_active;
    
    User() : is_active(0) {
        memset(userName, 0, sizeof(userName));
        memset(password, 0, sizeof(password));
    }
    
    User(const string& uname, const string& pwd) 
        : is_active(1) {
        strncpy(userName, uname.c_str(), sizeof(userName)-1);
        strncpy(password, pwd.c_str(), sizeof(password)-1);
    }
};

// File entry structure
struct FileEntry {
    char name[MAX_FILENAME_LEN+1];
    char userName[MAX_USERNAME_LEN+1];
    uint8_t is_valid;
    uint32_t size;
    uint32_t inode;      // First block index in disk.omni
    
    FileEntry() : is_valid(0), size(0), inode(0) {
        memset(name, 0, sizeof(name) - 1);
        memset(userName, 0, sizeof(userName) - 1);
    }
};

// Block header for disk storage
struct BlockHdr {
    uint32_t nxt;       // Next block in chain
    uint8_t isValid;    // 1 = valid, 0 = free
    uint32_t size;      // Size of data in this block
    
    BlockHdr() : nxt(0), isValid(1), size(BLOCK_SIZE - sizeof(BlockHdr)) {}
};
#endif