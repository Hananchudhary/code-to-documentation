#include <iostream>
#include <vector>
#include <string>
#include<cstdint>
#include <cstring>
#include"Btree.h"
using namespace std;
uint32_t myHash(const string& pass){
    int h = 1;
    int s = pass.size();
    for(int i = 0;i<s;i++){
        h = (h*(pass[i] - 31))%MAX_USERS;
    }
    return h;
}
class UserDBManager{
    BTree user_idx;
    vector<User> users;
    fstream file;
    public:
    UserDBManager(): user_idx(UserDB_FILE, 4, MAX_USERS){
        users.resize(MAX_USERS);
        file.open(UserDB_FILE, std::ios::binary | std::ios::in | std::ios::out);
    }
    int addUser(const string& username,const string& password){
        uint32_t idx = myHash(username);
        if(!user_idx.search(idx)){
            user_idx.insert(idx);
            file.seekp(idx * sizeof(User) + MAX_USERS, ios::beg);
            User u(username, password);
            file.write(reinterpret_cast<const char*>(&u), sizeof(User));
            users[idx] = u;
        }
        return 0;
    }
    User getUser(const string& username) {
        uint32_t idx = myHash(username);
        if (!user_idx.search(idx)) throw runtime_error("User not found");
        User u;
        file.seekg(idx * sizeof(User) + MAX_USERS, ios::beg);
        file.read(reinterpret_cast<char*>(&u), sizeof(User));
        return u;
    }
};
int main(){
    {
        UserDBManager ud;
        ud.addUser("hanan", "hanan");
    }
    UserDBManager ud;
    User u = ud.getUser("hanan");
    cout << u.userName << endl;
    return 0;
}