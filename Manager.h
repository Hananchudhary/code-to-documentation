#pragma once
#include"iostream"
#include"files_db.h"
#include"users_db.h"
#include"parser.h"
#include"trie.h"
#include"maxHeap.h"
using namespace std;
struct Order{
    uint64_t lastusage;
    string username;
    string filename;
    Order() = default;
    Order(string f, string u):filename(f),username(u), lastusage{static_cast<uint64_t>(time(nullptr))}{}
};
class Manager{
    FileDBManager fdm;
    UserDBManager udm;
    Parser* p;
    trie dictionary[MAX_USERS];
    vector<vector<Order>> files;
    maxHeap<Order> mh;
    public:
    Manager(){
        files.resize(MAX_USERS, vector<Order>(MAX_USER_FILEs));
    }
    int logIn(const string username, const string password){
        User u;
        if(udm.getUser(username, password, u)){
            vector<string> res = fdm.getFilesofUser(username);
            int idx = myHash(username);
            for(int i = 0;i<res.size();i++){    
                Order* o = new Order(res[i], username);
                files[idx].push_back(*o);
                dictionary[idx].insert(res[i]);
            }
            mh.heapify(files[idx].data(), files[idx].size());
            return 0;
        }
        int err = udm.addUser(username, password);
        if(err == 0){
            vector<string> res = fdm.getFilesofUser(username);
            int idx = myHash(username);
            for(int i = 0;i<res.size();i++){
                Order* o = new Order(res[i], username);
                files[idx].push_back(*o);
                dictionary[idx].insert(res[i]);
            }
            mh.heapify(files[idx].data(), files[idx].size());
        }
        return err;
    }
    int createFile(const string filename, const string username, const string data){
        int err = fdm.createFile(username, filename, data);
        if(err == 0){
            int idx = myHash(username);
            dictionary[idx].insert(filename);
            Order* o = new Order(filename, username);
            files[idx].push_back(*o);
            mh.heapify(files[idx].data(), files[idx].size());
        }
        return err;
    }
    int readFile(const string filename, const string username, string& data){
        int err = fdm.ReadFile(username, filename, data);
        if(err == 0){
            int idx = myHash(username);
            dictionary[idx].insert(filename);
            Order* o = new Order(filename, username);
            files[idx].push_back(*o);
            mh.heapify(files[idx].data(), files[idx].size());
        }
        return err;
    }
    int editFile(const string filename, const string username, const string data){
        int err = fdm.editFile(username, filename, data);
        if(err == 0){
            int idx = myHash(username);
            dictionary[idx].insert(filename);
            Order* o = new Order(filename, username);
            files[idx].push_back(*o);
            mh.heapify(files[idx].data(), files[idx].size());
        }
        return err;
    }
    int generateDoc(const string filename, const string username, string& data){
        int err = fdm.ReadFile(username, filename, data);
        if(err!=0) return err;
        int idx = myHash(username);
        dictionary[idx].insert(filename);
        Order* o = new Order(filename, username);
        files[idx].push_back(*o);
        mh.heapify(files[idx].data(), files[idx].size());
        p = new Parser();
        data = p->parseTree(data);
        delete p;
        p=nullptr;
        return 0;
    }
    vector<string> getFileNames(const string username, const string pre){
        int idx = myHash(username);
        return dictionary[idx].wordSuggestTool(pre);
    }
    vector<string> getFiles(const string username){
        int idx = myHash(username);
        vector<string> res;
        for(int i = 0;i<files[idx].size();i++){
            res.push_back(files[idx][i].filename);
        }
        return res;
    }
};