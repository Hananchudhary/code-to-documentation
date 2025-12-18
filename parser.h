#pragma once
#include<iostream>
#include<vector>
#include"node_types.h"
#include<string>
#include<sstream>
#include<cstdint>
#include<algorithm>

using namespace std;

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        token = trim(token);
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

bool isDataType(const string& token) {
    vector<string> types = {"int", "float", "double", "char", "bool", "void", "string", "long", "short"};
    return find(types.begin(), types.end(), token) != types.end();
}

class Parser{
    Node* root;
    int currentLine;
    
    Node* parseDeclaration(const string& line) {
        Declaration* decl = new Declaration(currentLine);
        
        string cleanLine = line;
        if (cleanLine.back() == ';') cleanLine.pop_back();
        
        vector<string> parts = split(cleanLine, ' ');
        if (parts.empty()) return decl;
        
        string dataType = parts[0];
        
        if (parts.size() > 1) {
            string varPart = parts[1];
            for (size_t i = 2; i < parts.size(); i++) {
                varPart += " " + parts[i];
            }
            
            vector<string> varDecls = split(varPart, ',');
            
            for (const string& varDecl : varDecls) {
                variable var;
                var.dataType = dataType;
                var.scope = "local";
                var.MemType = "auto";
                
                size_t eqPos = varDecl.find('=');
                if (eqPos != string::npos) {
                    var.name = trim(varDecl.substr(0, eqPos));
                    var.val = trim(varDecl.substr(eqPos + 1));
                } else {
                    var.name = trim(varDecl);
                    var.val = "uninitialized";
                }
                
                decl->vars.push_back(var);
            }
        }
        
        return decl;
    }
    
    Node* parseExpression(const string& line) {
        Expression* expr = new Expression(currentLine);
        
        string cleanLine = line;
        if (cleanLine.back() == ';') cleanLine.pop_back();
        
        size_t eqPos = cleanLine.find('=');
        if (eqPos != string::npos) {
            string leftStr = trim(cleanLine.substr(0, eqPos));
            string rightStr = trim(cleanLine.substr(eqPos + 1));
            
            vector<string> leftParts = split(leftStr, ' ');
            if (leftParts.size() >= 2 && isDataType(leftParts[0])) {
                
                return parseDeclaration(line);
            }
            
            val* leftVal = new val();
            leftVal->name = leftStr;
            leftVal->val = leftStr;
            expr->leftOpr = leftVal;
            
            expr->operation = "=";
            
            val* rightVal = new val();
            rightVal->name = rightStr;
            rightVal->val = rightStr;
            expr->right = rightVal;
        }
        
        return expr;
    }
    
    Node* parseReturn(const string& line) {
        Node* returnNode = new Node(currentLine, "Return");
        
        size_t returnPos = line.find("return");
        if (returnPos != string::npos) {
            string value = trim(line.substr(returnPos + 6));
            if (value.back() == ';') value.pop_back();
            returnNode->name = "Return(" + value + ")";
        }
        
        return returnNode;
    }

public:
    Parser(): root{nullptr}, currentLine{1}{}
    
    Node* Parse(const string& code) {
        istringstream stream(code);
        string line;
        
        while (getline(stream, line)) {
            currentLine++;
            line = trim(line);
            
            if (line.empty() || line.find("#include") == 0 || 
                line.find("using namespace") == 0) {
                continue;
            }
            
            if (line.find("int main()") == 0) {
                FunctionDef* mainFunc = new FunctionDef(currentLine);
                mainFunc->returnType = "int";
                mainFunc->funcName = "main";
                
                FunctionBody* body = new FunctionBody(currentLine);
                mainFunc->body = body;
                
                int braceCount = 0;
                size_t bracePos = line.find('{');
                if (bracePos != string::npos) {
                    braceCount = 1;
                } else {
                    getline(stream, line);
                    currentLine++;
                    line = trim(line);
                    if (line == "{") braceCount = 1;
                }
                
                while (braceCount > 0 && getline(stream, line)) {
                    currentLine++;
                    line = trim(line);
                    
                    if (line.empty()) continue;
                    
                    if (line == "{") {
                        braceCount++;
                        continue;
                    } else if (line == "}") {
                        braceCount--;
                        if (braceCount == 0) break;
                        continue;
                    }
                    
                    Node* stmtNode = nullptr;
                    
                    if (line.find("int ") == 0) {
                        stmtNode = parseDeclaration(line);
                    } else if (line.find("return") == 0) {
                        stmtNode = parseReturn(line);
                    } else if (line.find('=') != string::npos) {
                        stmtNode = parseExpression(line);
                    }
                    
                    if (stmtNode) {
                        body->statements.push_back(stmtNode);
                    }
                }
                
                if (!body->statements.empty()) {
                    body->root = body->statements[0];
                    Node* current = body->root;
                    for (size_t i = 1; i < body->statements.size(); i++) {
                        current->child = body->statements[i];
                        current = body->statements[i];
                    }
                }
                
                root = mainFunc;
                return root;
            }
        }
        
        return nullptr;
    }
    
    void printTree() {
        if (!root) {
            cout << "No AST generated." << endl;
            return;
        }
        
        cout << "Root" << endl;
        cout << "|" << endl;
        
        if (root->type == "FunctionDef") {
            FunctionDef* func = (FunctionDef*)root;
            
            cout << "|- Function definition" << endl;
            cout << "   |" << endl;
            cout << "   | - Name(\"" << func->funcName << "\")" << endl;
            cout << "   | - ReturnType(\"" << func->returnType << "\")" << endl;
            cout << "   | - Function Body" << endl;
            
            if (func->body && func->body->type == "FunctionBody") {
                FunctionBody* body = (FunctionBody*)func->body;
                
                cout << "       |" << endl;
                
                Node* current = body->root;
                while (current) {
                    cout << "       | - ";
                    
                    if (current->type == "Declaration") {
                        Declaration* decl = (Declaration*)current;
                        cout << "Declaration(";
                        for (size_t i = 0; i < decl->vars.size(); i++) {
                            cout << decl->vars[i].name;
                            if (decl->vars[i].val != "uninitialized") {
                                cout << " = " << decl->vars[i].val;
                            }
                            if (i != decl->vars.size() - 1) cout << ", ";
                        }
                        cout << ")";
                    }
                    else if (current->type == "Expression") {
                        Expression* expr = (Expression*)current;
                        if (expr->leftOpr && expr->right) {
                            if (expr->leftOpr->dataType != "") {
                                cout << "Declaration(" << expr->leftOpr->name << "), ";
                            } else {
                                cout << "variable: '" << expr->leftOpr->name << "', ";
                            }
                            cout << "opr: '" << expr->operation << "', ";
                            cout << "right: variable: '" << expr->right->name << "'";
                        }
                    }
                    else if (current->type == "Return") {
                        cout << "returned (" << current->name.substr(7, current->name.size()-8) << ")";
                    }
                    
                    cout << endl;
                    current = current->child;
                }
            }
        }
        
    }
};
