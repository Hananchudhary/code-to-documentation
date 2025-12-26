#pragma once
#include<iostream>
#include"Btree.h"
using namespace std;
uint32_t myHash(const string& pass, const string& username){
    int h = 1;
    int s = pass.size();
    int i = 0;
    while(i<pass.size() && i < username.size()){
        h = h*(pass[i] + username[i] - 31) % MAX_FILES;
    }
    for(int i = 0;i<s;i++){
        h = (h*(pass[i] - 31))%MAX_USERS;
    }
    return h;
}
class FileDBManager{

};