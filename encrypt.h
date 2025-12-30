#ifndef ENCRYPT_H
#define ENCRYPT_H
#include<iostream>
#include<vector>
#include<algorithm>
#include<string>
#include<string.h>
using namespace std;

class Encrypt{
    vector<char> encrypt_map;
    vector<char> decrypt_map;
    int size;
    
public:
    Encrypt(){
        string r = "qwertyuiopasdfghjkl;1234567890zxcvbnm,./][\\;'{}|:\"<>+_!@#$%^&*()ASDFGHJKLZXCVBNMOPQWERTYUI";
        string h = "asdfghjkl;qwertyuiopzxcvbnm,./1234567890{}|:\"][\\;'+_<>)(*&^%$#@!ZXCVBNMOPASDFGHJKLTYUIQWER";
        
        size = 256; // Use full ASCII range to be safe
        encrypt_map.resize(size);
        decrypt_map.resize(size);
        
        // Initialize both maps
        for(int i = 0; i < size; i++) {
            encrypt_map[i] = i; // Default: map to itself
            decrypt_map[i] = i; // Default: map to itself
        }
        
        // Build encryption and decryption maps
        for(int i = 0; i < r.size() && i < h.size(); i++) {
            int from_char = static_cast<unsigned char>(r[i]);
            int to_char = static_cast<unsigned char>(h[i]);
            
            encrypt_map[from_char] = to_char;
            decrypt_map[to_char] = from_char;
        }
    }
    
    void encrypt(char* buffer, size_t s) {
        for(size_t i = 0; i < s; i++) {
            unsigned char current_char = static_cast<unsigned char>(buffer[i]);
            buffer[i] = encrypt_map[current_char];
        }
    }
    
    void decrypt(char* buffer, size_t s) {
        for(size_t i = 0; i < s; i++) {
            unsigned char current_char = static_cast<unsigned char>(buffer[i]);
            buffer[i] = decrypt_map[current_char];
        }
    }
    
    template<typename T>
    void serializeEncrypt(const T& obj, char* buffer) {
        std::memcpy(buffer, &obj, sizeof(T));
        encrypt(buffer, sizeof(T));
    }
    
    template<typename T>
    void decryptDeserialize(char* buffer, T& obj) {
        decrypt(buffer, sizeof(T));
        std::memcpy(&obj, buffer, sizeof(T));
    }
};
#endif