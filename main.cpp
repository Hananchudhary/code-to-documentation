#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<cstdint>
#include<algorithm>
#include<fstream>
#include"users_db.h"
#include"parser.h"
#include"files_db.h"
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

int main() {
    system("fallocate -l 2M files.omni");
    FileDBManager fdm;

    // cout << "=== FILE DB PERSISTENCY TEST ===\n";

    // // ---------- CREATE FILES ----------
    // cout << "\n[Creating files]\n";

    // fdm.createFile("alice", "readme.txt",
    //     "Hello, this is Alice's file.\nPersistent storage test.");

    // // fdm.createFile("bob", "notes.txt","Bob's notes:\n- OS\n- DBMS\n- File Systems");

    // // fdm.createFile("charlie", "todo.txt","TODO:\n1. Finish project\n2. Sleep\n3. Debug");

    // cout << "File creation attempted.\n";

    // ---------- READ FILES ----------
    cout << "\n[Reading files]\n";

    FileEntry fe;
    string data;

    if (fdm.ReadFile("alice", fe, "readme.txt", data) == 0) {
        cout << "\nAlice/readme.txt:\n" << data << endl;
    } else {
        cout << "Failed to read Alice/readme.txt\n";
    }

    if (fdm.ReadFile("bob", fe, "notes.txt", data) == 0) {
        cout << "\nBob/notes.txt:\n" << data << endl;
    } else {
        cout << "Failed to read Bob/notes.txt\n";
    }

    if (fdm.ReadFile("charlie", fe, "todo.txt", data) == 0) {
        cout << "\nCharlie/todo.txt:\n" << data << endl;
    } else {
        cout << "Failed to read Charlie/todo.txt\n";
    }

    cout << "\n=== END OF TEST ===\n";

    return 0;
}
