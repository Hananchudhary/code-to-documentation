#pragma once
#include <iostream>
#include <vector>
#include <string>
#include<cstdint>
#include <cstring>
#include"Btree.h"
#include"AVL.h"
using namespace std;
uint32_t myHash(const string& pass){
    int h = 1;
    int s = pass.size();
    for(int i = 0;i<s;i++){
        h = (h*(pass[i] - 31))%MAX_USERS;
    }
    return h;
}
const int offset = (MAX_USERS * sizeof(BTreeNode<4,MAX_USERS, 0>)) + (MAX_USERS/8);
class UserDBManager{
    BTree<4,MAX_USERS, 0> user_idx;
    vector<AVLTree<User>> users;
    DiskManager<AVLNode<User>, MAX_USERS, offset> dm;
    fstream file;
    int size;
    public:
    UserDBManager(): user_idx(UserDB_FILE, 4, MAX_USERS), size{0}, dm(UserDB_FILE){
        users.resize(MAX_USERS);
        file.open(UserDB_FILE, std::ios::binary | std::ios::in | std::ios::out);
    }
    int addUser(const string& username,const string& password){
        uint32_t idx = myHash(username);
        int i = idx;
        if(size>MAX_USERS) return static_cast<int>(OFSErrorCodes::ERROR_MAX_USERS);
        if(!users[idx].search(username)){
            if(!user_idx.search(idx)){
                user_idx.insert(idx);
                dm.setBlock(idx);
            }
            else{
                i = dm.allocateBlock();
            } 
            User u(username, password);
            users[idx].insert(username, u,file, (i*sizeof(AVLNode<User>)+offset+MAX_USERS/8));
            size++;
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
        return static_cast<int>(OFSErrorCodes::ERROR_USER_EXISTS);
    }
    bool getUser(const string& username, User& u) {
        uint32_t idx = myHash(username);
        if (!user_idx.search(idx)) return false;
        AVLNode<User>* us = users[idx].search(username);
        if(!us){
            users[idx].loadTree(file,(MAX_USERS/8 + offset + idx*sizeof(AVLNode<User>)));
        }
        us = users[idx].search(username);
        if(!us) return false;
        return true;
    }
    // int deleteUser(const string& username,const string& password){

    // }
};