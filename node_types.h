#pragma once
#include<iostream>
#include<cstdint>
#include<memory.h>
#include"config.h"
using namespace std;
struct User {
    char id[32];        // Fixed size for B-tree
    char name[64];
    char password[64];
    uint8_t is_active;
    
    User() : is_active(0) {
        memset(id, 0, sizeof(id));
        memset(name, 0, sizeof(name));
        memset(password, 0, sizeof(password));
    }
    
    User(const string& uid, const string& uname, const string& pwd) 
        : is_active(1) {
        strncpy(id, uid.c_str(), sizeof(id)-1);
        strncpy(name, uname.c_str(), sizeof(name)-1);
        strncpy(password, pwd.c_str(), sizeof(password)-1);
    }
};

// File entry structure
struct FileEntry {
    char id[32];
    char name[256];
    char user_id[32];
    uint8_t is_valid;
    uint32_t size;
    uint32_t inode;      // First block index in disk.omni
    
    FileEntry() : is_valid(0), size(0), inode(0) {
        memset(id, 0, sizeof(id));
        memset(name, 0, sizeof(name));
        memset(user_id, 0, sizeof(user_id));
    }
};

// Block header for disk storage
struct BlockHdr {
    uint32_t nxt;       // Next block in chain
    uint8_t isValid;    // 1 = valid, 0 = free
    uint32_t size;      // Size of data in this block
    
    BlockHdr() : nxt(0), isValid(1), size(BLOCK_SIZE - sizeof(BlockHdr)) {}
};
