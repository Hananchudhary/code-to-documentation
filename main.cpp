#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<cstdint>
#include<algorithm>
#include<fstream>
#include"users_db.h"
#include"parser.h"

using namespace std;

string readInputFromFile(string filename){
    ifstream fin(filename);
    string code;
    while(!fin.eof()){
        char buffer[4096]{};
        fin.getline(buffer, 4096, EOF);
        code = code + string(buffer);
    }
    return code;
}
void firstRun() {
    UserDBManager db;

    db.addUser("alice", "123");
    db.addUser("bob", "456");
    db.addUser("charlie", "789");

    cout << "[RUN 1] Users inserted\n";
}

void secondRun() {
    UserDBManager db;
    User u;

    cout << "[RUN 2] Reading persisted users\n";

    if (db.getUser("alice", u))
        cout << "alice OK inode=" << endl;
    else
        cout << "alice FAIL\n";

    if (db.getUser("bob", u))
        cout << "bob OK inode="<< endl;
    else
        cout << "bob FAIL\n";

    if (db.getUser("charlie", u))
        cout << "charlie OK inode=" << endl;
    else
        cout << "charlie FAIL\n";
}

int main() {
    cout << "1 = insert, 2 = read\n> ";
    int mode;
    cin >> mode;

    if (mode == 1)
        firstRun();
    else
        secondRun();

    return 0;
}
