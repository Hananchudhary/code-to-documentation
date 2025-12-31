#ifndef NODE_TYPES_H
#define NODE_TYPES_H
#include<iostream>
#include<cstdint>
#include<memory.h>
#include<vector>
#include"config.h"
using namespace std;
enum class OFSErrorCodes : int32_t {
    SUCCESS = 0,                      // Operation completed successfully
    ERROR_MAX_USERS = -12,
    ERROR_USER_EXISTS = -13,
    ERROR_NOT_FOUND = -1,            // File/directory/user not found
    ERROR_PERMISSION_DENIED = -2,    // User lacks required permissions
    ERROR_IO_ERROR = -3,             // File I/O operation failed
    ERROR_INVALID_PATH = -4,         // Path format is invalid
    ERROR_FILE_EXISTS = -5,          // File/directory already exists
    ERROR_NO_SPACE = -6,             // Insufficient space in file system
    ERROR_INVALID_CONFIG = -7,       // Configuration file is invalid
    ERROR_NOT_IMPLEMENTED = -8,      // Feature not yet implemented
    ERROR_INVALID_SESSION = -9,      // Session is invalid or expired
    ERROR_DIRECTORY_NOT_EMPTY = -10, // Cannot delete non-empty directory
    ERROR_INVALID_OPERATION = -11    // Operation not allowed
};
enum class Permission : uint8_t{
    OWN = 1,
    READ = 2
};
struct User {     
    char userName[MAX_USERNAME_LEN+1];
    char password[MAX_USERNAME_LEN+1];
    uint8_t is_active;
    uint16_t files;
    User() : is_active(0) {
        memset(userName, 0, sizeof(userName));
        memset(password, 0, sizeof(password));
    }
    
    User(const string& uname, const string& pwd) 
        :files{0}, is_active(1) {
        strncpy(userName, uname.c_str(), sizeof(userName)-1);
        strncpy(password, pwd.c_str(), sizeof(password)-1);
    }
};

struct FileEntry {
    char name[MAX_FILENAME_LEN+1];
    char userName[MAX_USERNAME_LEN+1];
    uint8_t is_valid;
    uint32_t size;
    uint32_t inode;      // First block index in disk.omni
    
    FileEntry() : is_valid(0), size(0), inode(0) {
        memset(name, 0, sizeof(name) - 1);
        memset(userName, 0, sizeof(userName) - 1);
    }
    FileEntry(const string& uname, const string& fname, uint32_t i) 
        :size{0}, is_valid(1), inode{i} {
        strncpy(userName, uname.c_str(), sizeof(userName)-1);
        strncpy(name, fname.c_str(), sizeof(fname)-1);
    }
};

// Block header for disk storage
struct BlockHdr {
    uint32_t nxt;       // Next block in chain
    uint8_t isValid;    // 1 = valid, 0 = free
    uint32_t size;      // Size of data in this block
    
    BlockHdr() : nxt(0), isValid(1), size(BLOCK_SIZE - sizeof(BlockHdr)) {}
};
struct Node{
    int line;
    string name;
    string type;
    Node* child;
    Node(int l = 0, string t = "Node"):line{l}, name{"unknown"}, type{t}, child{nullptr}{} 
};

struct val{
    string dataType;
    string val;
    string name;
};

struct variable : val{
    string scope;
    string MemType;
};

struct Declaration: Node{
    vector<variable> vars; 
    Declaration(int l):Node(l, "Declaration"){
        name = "Declaration";
    }
};

struct Expression : Node{
    val* leftOpr;
    string operation;
    val* right;
    Expression(int l):Node(l, "Expression"){
        name = "Expression";
        leftOpr = nullptr;
        operation = "";
        right = nullptr;
    }
};

struct FunctionDef: Node{
    string returnType;
    string funcName;
    vector<val*> params;
    Node* body;
    FunctionDef(int l):Node(l, "FunctionDef"){
        name = "FunctionDef";
        body = nullptr;
        funcName = "main";
    }
};

struct FunctionBody: Node{
    Node* root;
    vector<Node*> statements;
    FunctionBody(int l):Node(l, "FunctionBody"){
        name = "FunctionBody";
        root = nullptr;
    }
};
struct IfStatement : Node {
    string condition;
    Node* thenBody;
    Node* elseBody;

    IfStatement(int l) : Node(l, "IfStatement") {
        name = "IfStatement";
        thenBody = nullptr;
        elseBody = nullptr;
    }
};

struct ForLoop : Node {
    string init;
    string condition;
    string increment;
    Node* body;

    ForLoop(int l) : Node(l, "ForLoop") {
        name = "ForLoop";
        body = nullptr;
    }
};

struct Program : Node {
    vector<FunctionDef*> functions;
    Program() : Node(0, "Program") {
        name = "Program";
    }
};
#endif