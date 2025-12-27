#ifndef CONFIG_H
#define CONFIG_H
#include<iostream>
using namespace std;
const int MAX_USERS = 24;
const int BLOCK_SIZE = 4096;
const int MAX_BLOCKS = 10000;
const int MAX_FILES = 1000;
char UserDB_FILE[] = "users.omni";
char FilesDB_FILE[] = "files.omni";
const int MAX_USERNAME_LEN = 65;
const int MAX_FILENAME_LEN = 65;
const int MAX_USER_FILEs = 35;
const int MAX_CONNECTIONS = 50;
const int NUM_WORKERS = 1;
#endif