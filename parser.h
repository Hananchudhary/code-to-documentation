#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "node_types.h"

using namespace std;

/* ==================== Utilities ==================== */

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

vector<string> split(const string& s, char delim) {
    vector<string> tokens;
    string tok;
    istringstream ss(s);
    while (getline(ss, tok, delim))
        tokens.push_back(trim(tok));
    return tokens;
}

bool isDataType(const string& s) {
    static vector<string> types = {
        "int","float","double","char","bool","void","string"
    };
    return find(types.begin(), types.end(), s) != types.end();
}

/* ==================== Parser ==================== */

class Parser {
    Program* root;
    int currentLine;

    /* ---------- Statement Parsers ---------- */

    Node* parseDeclaration(const string& line) {
        Declaration* decl = new Declaration(currentLine);

        string clean = line;
        if (clean.back() == ';') clean.pop_back();

        auto parts = split(clean, ' ');
        string type = parts[0];

        auto vars = split(clean.substr(type.size()), ',');

        for (auto& v : vars) {
            variable var;
            var.dataType = type;
            var.scope = "local";
            var.MemType = "auto";

            size_t eq = v.find('=');
            if (eq != string::npos) {
                var.name = trim(v.substr(0, eq));
                var.val = trim(v.substr(eq + 1));
            } else {
                var.name = trim(v);
                var.val = "uninitialized";
            }
            decl->vars.push_back(var);
        }
        return decl;
    }

    Node* parseExpression(const string& line) {
        Expression* expr = new Expression(currentLine);

        string clean = line;
        if (clean.back() == ';') clean.pop_back();

        size_t eq = clean.find('=');
        if (eq == string::npos) return expr;

        expr->leftOpr = new val();
        expr->leftOpr->name = trim(clean.substr(0, eq));

        expr->operation = "=";

        expr->right = new val();
        expr->right->name = trim(clean.substr(eq + 1));

        return expr;
    }

    Node* parseReturn(const string& line) {
        Node* r = new Node(currentLine, "Return");
        string val = trim(line.substr(6));
        if (!val.empty() && val.back() == ';') val.pop_back();
        r->name = "returned (" + val + ")";
        return r;
    }

    IfStatement* parseIf(istringstream& stream, string line) {
        IfStatement* ifs = new IfStatement(currentLine);

        size_t l = line.find('('), r = line.find(')');
        ifs->condition = line.substr(l + 1, r - l - 1);

        ifs->thenBody = parseBlock(stream);

        stream >> ws;
        streampos pos = stream.tellg();
        string next;
        getline(stream, next);
        next = trim(next);

        if (next.find("else") == 0) {
            ifs->elseBody = parseBlock(stream);
        } else {
            stream.seekg(pos);
        }
        return ifs;
    }

    ForLoop* parseFor(istringstream& stream, string line) {
        ForLoop* loop = new ForLoop(currentLine);

        size_t l = line.find('('), r = line.find(')');
        auto parts = split(line.substr(l + 1, r - l - 1), ';');

        loop->init = parts[0];
        loop->condition = parts[1];
        loop->increment = parts[2];

        loop->body = parseBlock(stream);
        return loop;
    }

    /* ---------- Block Parser ---------- */

    Node* parseBlock(istringstream& stream) {
        FunctionBody* body = new FunctionBody(currentLine);
        string line;
        int braces = 0;

        while (getline(stream, line)) {
            currentLine++;
            line = trim(line);

            if (line == "{") { braces++; continue; }
            if (line == "}") {
                if (--braces < 0) break;
                continue;
            }

            Node* stmt = nullptr;

            if (line.find("if") == 0)
                stmt = parseIf(stream, line);
            else if (line.find("for") == 0)
                stmt = parseFor(stream, line);
            else if (line.find("return") == 0)
                stmt = parseReturn(line);
            else if (isDataType(split(line, ' ')[0]))
                stmt = parseDeclaration(line);
            else if (line.find('=') != string::npos)
                stmt = parseExpression(line);

            if (stmt)
                body->statements.push_back(stmt);
        }

        if (!body->statements.empty()) {
            body->root = body->statements[0];
            Node* cur = body->root;
            for (size_t i = 1; i < body->statements.size(); i++) {
                cur->child = body->statements[i];
                cur = body->statements[i];
            }
        }
        return body;
    }

    /* ---------- Function Parser ---------- */

    FunctionDef* parseFunction(istringstream& stream, string header) {
        FunctionDef* fn = new FunctionDef(currentLine);

        auto parts = split(header, ' ');
        fn->returnType = parts[0];
        fn->funcName = parts[1].substr(0, parts[1].find('('));

        fn->body = parseBlock(stream);
        return fn;
    }
    void printStatementChain(Node* node, string indent, string& out) {
        while (node) {
            out += indent + "| - ";

            if (node->type == "Declaration") {
                Declaration* d = (Declaration*)node;
                if(!d){
                    out += "\n";
                    node = node->child;
                }
                out += "Declaration(";
                for (size_t i = 0; i < d->vars.size(); i++) {
                    out += d->vars[i].name;
                    if (d->vars[i].val != "uninitialized")
                        out += " = " + d->vars[i].val;
                    if (i + 1 < d->vars.size()) out += ", ";
                }
                out += ")";
            }
            else if (node->type == "Expression") {
                Expression* e = (Expression*)node;
                out += "Expression(left: variable: '" + e->leftOpr->name +
                       "', opr: '" + e->operation +
                       "', right: variable: '" + e->right->name + "')";
            }
            else if (node->type == "Return") {
                out += node->name;  // already "returned (x)"
            }
            else if (node->type == "IfStatement") {
                IfStatement* i = (IfStatement*)node;
                out += "If(condition: " + i->condition + ")\n";

                out += indent + "   | - Then\n";
                printStatementChain(i->thenBody->child, indent + "   |   ", out);

                if (i->elseBody) {
                    out += indent + "   | - Else\n";
                    printStatementChain(i->elseBody->child, indent + "   |   ", out);
                }
            }
            else if (node->type == "ForLoop") {
                ForLoop* f = (ForLoop*)node;
                out += "For(init: " + f->init +
                       ", condition: " + f->condition +
                       ", increment: " + f->increment + ")\n";

                printStatementChain(f->body->child, indent + "   |   ", out);
            }

            out += "\n";
            node = node->child;
        }
    }
    Node* Parse(const string& code) {
        istringstream stream(code);
        string line;
        while (getline(stream, line)) {
            currentLine++;
            line = trim(line);
            if (line.empty()) continue;
            auto parts = split(line, ' ');
            if (parts.size() >= 2 && isDataType(parts[0]) && line.find('(') != string::npos) {
                root->functions.push_back(parseFunction(stream, line));
            }
        }
        return root;
    }

public:
    Parser() : root(new Program()), currentLine(0) {}

    
    string parseTree(const string& code) {
        string out;
        Node* ast = Parse(code);

        if (!ast) {
            return "No AST generated.\n";
        }

        Program* program = (Program*)ast;

        out += "Root\n";
        out += "|\n";

        for (auto* func : program->functions) {
            out += "|- Function definition\n";
            out += "   |\n";
            out += "   | - Name(\"" + func->funcName + "\")\n";
            out += "   | - returnType(\"" + func->returnType + "\")\n";
            out += "   | - Function Body\n";
            out += "       |\n";

            if (func->body && func->body->type == "FunctionBody") {
                FunctionBody* body = (FunctionBody*)func->body;
                if(body)
                    printStatementChain(body->root, "       ", out);
            }
        }

        return out;
    }

};
