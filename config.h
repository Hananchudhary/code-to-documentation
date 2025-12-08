#pragma once
#include<iostream>
using namespace std;
const int MAX_USERS = 50;
const int BLOCK_SIZE = 4096;
const int MAX_BLOCKS = 10000; // Adjust based on disk size
const int MAX_FILES = 1000;
char DB_FILE[] = "disk.omni";