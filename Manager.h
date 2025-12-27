#pragma once
#include"iostream"
#include"files_db.h"
#include"users_db.h"
#include"parser.h"
using namespace std;
class Manager{
    FileDBManager fdm;
    UserDBManager udm;
    Parser* p;
    public:
    Manager() = default;
    int logIn(const string username, const string password){
        User u;
        if(udm.getUser(username, password, u))
            return 0;
        return udm.addUser(username, password);
    }
    int createFile(const string filename, const string username, const string data){
        return fdm.createFile(username, filename, data);
    }
    int readFile(const string filename, const string username, string& data){
        return fdm.ReadFile(username, filename, data);
    }
    int editFile(const string filename, const string username, const string data){
        return fdm.editFile(username, filename, data);
    }
    int generateDoc(const string filename, const string username, string& data){
        int err = fdm.ReadFile(username, filename, data);
        if(err!=0) return err;
        p = new Parser();
        data = p->parseTree(data);
        delete p;
        p=nullptr;
        return 0;
    }
};