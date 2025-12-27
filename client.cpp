#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

#define PORT 8080
#define BUFFER_SIZE 10000
#define SERVER_IP "127.0.0.1"

// Helper to send JSON request and receive response
json send_request(const json& request) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    string req_str = request.dump();
    send(sock, req_str.c_str(), req_str.size(), 0);

    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes <= 0) {
        close(sock);
        return json();
    }

    json response = json::parse(buffer);
    close(sock);
    return response;
}

int main() {

    json login_req = {
        {"operation", "login"},
        {"parameters", {
            {"username", "alice"},
            {"password", "alice123"}
        }}
    };
    json login_res = send_request(login_req);
    cout << "Login response:\n" << login_res.dump(4) << "\n\n";

    json create_req = {
        {"operation", "create_file"},
        {"parameters", {
            {"username", "alice"},
            {"filename", "test.txt"},
            {"data", "Hello world! This is a test file."}
        }}
    };
    json create_res = send_request(create_req);
    cout << "Create file response:\n" << create_res.dump(4) << "\n\n";

    json read_req = {
        {"operation", "read_file"},
        {"parameters", {
            {"username", "alice"},
            {"filename", "test.txt"}
        }}
    };
    json read_res = send_request(read_req);
    cout << "Read file response:\n" << read_res.dump(4) << "\n\n";

    // --- Test 4: Edit File ---
    json edit_req = {
        {"operation", "edit_file"},
        {"parameters", {
            {"username", "alice"},
            {"filename", "test.txt"},
            {"data", "Hello world! File has been updated!"}
        }}
    };
    json edit_res = send_request(edit_req);
    cout << "Edit file response:\n" << edit_res.dump(4) << "\n\n";

    // --- Test 5: Read Edited File ---
    json read_req2 = {
        {"operation", "read_file"},
        {"parameters", {
            {"username", "alice"},
            {"filename", "test.txt"}
        }}
    };
    json read_res2 = send_request(read_req2);
    cout << "Read edited file response:\n" << read_res2.dump(4) << "\n\n";

    return 0;
}
