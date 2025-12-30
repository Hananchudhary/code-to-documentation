#pragma once
#include<iostream>
#include<fstream>
#include"config.h"
#include"sAVL.h"
#include<cstring>
using namespace std;
class FSManager{
    sAVLTree<string> graph;
    public:
    FSManager(){
        const int BUFFER = MAX_FILENAME_LEN + (MAX_USERNAME_LEN * 2);
        int size =0;
        fstream file(ShareDB_FILE, ios::binary | ios::in | ios::out);
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        for(int i =0;i<size;i++){
            char STRING[BUFFER]={0};
            file.read(reinterpret_cast<char*>(STRING), BUFFER);
            string key(STRING);
            char FILE[MAX_FILENAME_LEN+MAX_USERNAME_LEN]={0};
            file.read(reinterpret_cast<char*>(FILE), MAX_FILENAME_LEN+MAX_USERNAME_LEN);
            string f(FILE);
            graph.insert(key, f);
        }
        file.close();
    }
    void share_file(const string key, const string filename){
        graph.insert(key, filename);
    }
    void delete_share(const string key){
        graph.remove(key);
    }
    bool hasAccess(const string key, string& filename){
        sAVLNode<string>* has = graph.search(key);
        if(!has) return false;
        filename = has->value;
        return true;
    }
    ~FSManager(){
        const int BUFFER = MAX_FILENAME_LEN + (MAX_USERNAME_LEN * 2);
        vector<string> edges = graph.getAllKeys();
        vector<string> vals = graph.getAllValues();
        int size = edges.size();
        fstream file(ShareDB_FILE, ios::binary | ios::in | ios::out);
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for(int i =0;i<size;i++){
            char STRING[BUFFER]={0};
            strncpy(STRING, &edges[i][0], edges[i].size());
            file.write(reinterpret_cast<const char*>(STRING), BUFFER);
            char FILE[MAX_FILENAME_LEN+MAX_USERNAME_LEN]={0};
            strncpy(FILE, &vals[i][0], vals[i].size());
            file.write(reinterpret_cast<const char*>(FILE), MAX_FILENAME_LEN+MAX_USERNAME_LEN);
        }
        file.close();
    }
};