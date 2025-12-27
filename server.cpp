#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<cstdint>
#include<algorithm>
#include<fstream>
#include"Manager.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <nlohmann/json.hpp>
#include"Huffman.h"
#include"circularqueue.h"
using json = nlohmann::json;
using namespace std;
#define PORT 8080
#define BUFFER_SIZE 10000
struct Req{
    int client_fd;
    string request;
    Req() = default;
    Req(int c, char buffer[BUFFER_SIZE], int b):client_fd{c}, request(buffer, b){
    }
};
Manager manager;
CircularQueue<Req> req_q(MAX_CONNECTIONS);
json make_success_response(const string& operation, const json data={}) {
    json response;
    response["status"] = "success";
    response["operation"] = operation;
    response["data"] = data;
    return response;
}

json make_error_response(const string& operation,
                         int error_code,
                         const string& error_message) {
    json response;
    response["status"] = "error";
    response["operation"] = operation;
    response["error_code"] = error_code;
    response["error_message"] = error_message;
    return response;
}

json handle_request(const json& req) {
    string operation = req.value("operation", "");
    json params = req.value("parameters", json::object());

    if (operation == "login") {
        string username = params["username"];
        string password = params["password"];

        int err = manager.logIn(username, password);
        if (err == 0) {
            return make_success_response(operation);
        }
        return make_error_response(operation, err, "Invalid credentials");
    }

    if (operation == "create_file") {
        string username = params["username"];
        string filename = params["filename"];
        string data = params["data"];

        int err = manager.createFile(filename, username, data);
        if (err == 0) {
            return make_success_response(operation);
        }
        return make_error_response(operation, err, "File creation failed");
    }

    if (operation == "read_file") {
        string username = params["username"];
        string filename = params["filename"];
        string data;

        int err = manager.readFile(filename, username, data);
        if (err == 0) {
            json res;
            res["result_data"] = data;
            return make_success_response(operation, res);
        }
        return make_error_response(operation, err, "Read failed");
    }

    if (operation == "edit_file") {
        string username = params["username"];
        string filename = params["filename"];
        string data = params["data"];

        int err = manager.editFile(filename, username, data);
        if (err == 0) {
            return make_success_response(operation );
        }
        return make_error_response(operation, err, "Edit failed");
    }

    if (operation == "generate_doc") {
        string username = params["username"];
        string filename = params["filename"];
        string data;

        int err = manager.generateDoc(filename, username, data);
        if (err != 0)
            return make_error_response(operation, err, "Document generation failed");

        HuffmanCoding hf;
        auto freq = countfreq(data);
        string encoded = hf.encode(data);

        json res;
        res["encoded_data"] = encoded;
        res["freq_table"] = freq;
        res["compression"] = "huffman";
        res["original_size"] = data.size();
        res["encoded_size"] = encoded.size();

        return make_success_response(operation, res);
    }
    if(operation == "get_files"){

    }
    return make_error_response(operation,-1, "Unknown operation");
}
void worker_thread() {
    Req r;
    while (true) {
        if(req_q.try_dequeue(r)){
            try {
                json request = json::parse(r.request);
                json response = handle_request(request);
                string response_str = response.dump();
                send(r.client_fd, response_str.c_str(), response_str.size(), 0);
                cout << "Handled request: " << response_str << endl;
            } catch (exception& e) {
                json err = make_error_response("unknown", -500, e.what());
                string err_str = err.dump();
                send(r.client_fd, err_str.c_str(), err_str.size(), 0);
            }
        }

        // close(r.client_fd);
    }
}
int main() {
    system("fallocate -l 100M files.omni");
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        return 1;
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        return 1;
    }
    vector<thread> workers;
    for (int i = 0; i < NUM_WORKERS; ++i) {
        workers.emplace_back(worker_thread);
    }
    cout << "Server listening on port " << PORT << endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        char buffer[BUFFER_SIZE] = {0};
        int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            // close(client_fd);
            continue;
        }
        Req* r = new Req(client_fd, buffer, bytes);
        req_q.try_enqueue(*r);
        // close(client_fd);
    }

    close(server_fd);
    return 0;
}