#pragma once
#include"iostream"
#include"files_db.h"
#include"users_db.h"
#include"parser.h"
#include"trie.h"
#include"sAVL.h"
#include"filesharing.h"
#include"maxHeap.h"
#include"encrypt.h"
using namespace std;
struct Order{
    uint64_t lastusage;
    string username;
    string filename;
    Order() = default;
    Order(string f, string u):filename(f),username(u), lastusage{static_cast<uint64_t>(time(nullptr))}{}
    bool operator<(const Order& obj){
        return this->lastusage<obj.lastusage;
    }
    bool operator<=(const Order& obj){
        return this->lastusage<=obj.lastusage;
    }
    bool operator>(const Order& obj){
        return this->lastusage>obj.lastusage;
    }
    bool operator>=(const Order& obj){
        return this->lastusage>=obj.lastusage;
    }
    bool operator==(const Order& obj){
        return this->lastusage==obj.lastusage;
    }
    bool operator!=(const Order& obj){
        return this->lastusage!=obj.lastusage;
    }
};
class Manager{
    FileDBManager fdm;
    UserDBManager udm;
    Parser* p;
    trie dictionary[MAX_USERS];
    vector<vector<Order>> files;
    maxHeap<Order> mh;
    FSManager fs;
    public:
    void remove(const string username, const string filename, vector<Order> f){
        for(int i =0;i<f.size();i++){
            if(strcmp(&f[i].filename[0], &filename[0])==0 && strcmp(&f[i].username[0], &username[0])==0){
                f.erase(f.begin()+i);
                break;
            }
        }
    }
    Manager(){
        files.resize(MAX_USERS, vector<Order>(MAX_USER_FILEs));
    }
    int share_file(const string username, const string filename, const string targetuser, string& k){
        string key(username+filename+targetuser);
        if(!fdm.hasFileAccess(username, filename)){
            return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
        }
        fs.share_file(key, username+filename);
        Encrypt em;
        k.resize(key.size());
        strncpy(&k[0], &key[0], key.size());
        em.encrypt(&k[0], k.size());
        return 0;
    }
    int ReadSharedFile(const string k, const string username, string& data){
        string filename;
        string key(k);
        Encrypt em;
        em.decrypt(&key[0], key.size());
        if(!fs.hasAccess(key, filename) || !fs.hasAccess(filename+username, filename)){
            return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
        }
        FileEntry f;
        if(!fdm.getUser(filename, f)){
            return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
        }
        return this->readFile(f.name, f.userName, data);
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
            remove(username, filename, files[idx]);
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
            remove(username, filename, files[idx]);
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
        remove(username, filename, files[idx]);
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
        vector<string> res = fdm.getFilesofUser(username);
        return res;
        for(int i = 0;i<files[idx].size();i++){
            if(files[idx][i].filename != "" && !has(res, files[idx][i].filename)){
                res.push_back(files[idx][i].filename);
            }
        }
        return res;
    }
    bool has(vector<string> arr, string t){
        for(int i =0;i<arr.size();i++){
            if(t == arr[i]){
                return true;
            }
        }
        return false;
    }
};