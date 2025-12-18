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
int main(){
    
    return 0;
}
int main1(){
    return 0;
}