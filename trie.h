#include<iostream>
#include<vector>
#include<stack>
using namespace std;
#define K_WAY 26
int getIndex(char ch){
    return (ch - 'a');
}
char getChar(int index){
    if(index < 0 || index >= K_WAY) return -1;
    return ('a' + index);
}
class trieNode
{
    trieNode* next[K_WAY]{};
    bool hasEnded;
    // You can add more attributes here as needed
    
public:
    trieNode(): hasEnded{false}{}
    void insert(char ch, bool state){
        int idx = getIndex(ch);
        next[idx] = new trieNode();
        hasEnded = state;
    }
    trieNode* getNode(char ch){
        return next[getIndex(ch)];
    }
    void setNode(int i, trieNode* node){
        next[i] = node;
    }
    bool isEnd(){
        return hasEnded;
    }
    void update(bool state){
        hasEnded = state;
    }
    ~trieNode() = default; // Trigger deletion of all branches from this node
};
class trie
{
    trieNode* root;
    
    void autoComplete(trieNode* node, string word, vector<string>& suggestions){
        string copi(word);
        for(int i = 0;i<K_WAY;i++){
            char ch = getChar(i);
            trieNode* temp = node->getNode(ch);
            if(temp){
                word = word + ch;
                if(temp->isEnd()){
                    suggestions.push_back(word);
                }
                autoComplete(temp, word, suggestions);
                word = copi;
            }
        }
    }
    bool isLeaf(trieNode* node){
        for(int i = 0;i<K_WAY;i++){
            if(node->getNode(getChar(i)))
                return false;
        }
        return true;
    }
    // Add all possible words from this branch into suggestions
    void del(trieNode* node, string key , int i, bool& flag){
        if(!node) return;
        if(i >= key.size() && node->isEnd()){
            node->update(false);
        }
        trieNode* n = node->getNode(key[i]);
        del(n, key, i+1, flag);
        if(n && isLeaf(n) && flag){
            if(n->isEnd()){
                flag = false;
            }
            else{
                int idx = getIndex(key[i]);
                delete n;
                node->setNode(idx, nullptr);
            }
        }
    }
    
public:
    trie() : root{new trieNode()}{}
    
    void insert(string key){
        int s = key.size();
        trieNode* ct = root;
        for(int i =0;i<s;i++){
            trieNode* temp = ct->getNode(key[i]);
            if(!temp){
                ct->insert(key[i], ct->isEnd());
                ct = ct->getNode(key[i]);
            }
            else
                ct = temp;
            cout << "";
        }
        ct->update(true);
    } // Insert key string into trie
    bool search(string key){
        int s = key.size();
        trieNode* ct = root;
        for(int i = 0;i<s;i++){
            trieNode* temp = ct->getNode(key[i]);
            if(!temp) return false;
            ct = temp;
        }
        if(ct->isEnd())
            return true;
        return false;
    } // Search for key string in trie
    void erase(string key){
        // int s = key.size();
        // trieNode* ct = root;
        // for(int i = 0;i<s;i++){
        //     trieNode* temp = ct->getNode(key[i]);
        //     ct = temp;
        // }
        // ct->update(false);
        bool flag = true;
        del(root, key, 0, flag);
        cout << "";
    } // Remove key string from trie
    ~trie(){
        stack<trieNode*> q;
        vector<trieNode*> visited;
        visited.push_back(root);
        q.push(root);
        while(!q.empty()){
            trieNode* tmp = q.top();
            q.pop();
            for(int i = 0;i<K_WAY;i++){
                char ch = getChar(i);
                trieNode* temp = tmp->getNode(ch);
                if(temp){
                    q.push(temp);
                    visited.push_back(temp);
                }
            }
        }
        int s = visited.size();
        for(int i =0;i<s;i++){
            delete[] visited[visited.size() - 1];
            visited.pop_back();
        }

    } // Deallocate entire trie
    
    vector<string> wordSuggestTool(string prefix){
        vector<string> sugg;
        string word(prefix);
        trieNode* ct = root;
        int s = prefix.size();
        for(int i =0;i<s;i++){
            trieNode* temp = ct->getNode(prefix[i]);
            if(!temp) break;
            ct = temp;
        }
        autoComplete(ct,word, sugg);
        return sugg;
    }
    // Return all words in trie that start with given prefix
};