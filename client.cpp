#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void readMessages(SOCKET client_socket) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break; // Соединение прервано или ошибка
        buffer[bytes_received] = '\0';
        cout << "Server: " << buffer << endl;
    }
}

void sendCommand(SOCKET client_socket, const string& command) {
    send(client_socket, command.c_str(), command.length(), 0);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(client_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed: " << WSAGetLastError() << endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to the server.\nEnter 'register' or 'login': ";
    string cmd, username, password;
    cin >> cmd;
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    string message = cmd + " " + username + " " + password;
    sendCommand(client_socket, message);

    char response[1024];
    int bytes_received = recv(client_socket, response, sizeof(response), 0);
    response[bytes_received] = '\0';

    if (strcmp(response, "success") != 0) {
        cout << "Error: " << response << endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    cout << "Successfully " << (cmd == "register" ? "registered." : "logged in.") << endl;

    thread receiver(readMessages, client_socket);
    receiver.detach();

    while (true) {
        cout << "Enter command (get_users, send_msg, or exit): ";
        cin >> cmd;
        if (cmd == "exit") {
            break;
        } else if (cmd == "get_users") {
            sendCommand(client_socket, "get_users");
        } else if (cmd == "send_msg") {
            string recipient;
            cout << "Enter recipient username: ";
            cin >> recipient;
            cin.ignore(); // Clear the buffer after reading input
            string content;
            cout << "Enter your message: ";
            getline(cin, content);
            message = "send_msg " + recipient + " " + content;
            sendCommand(client_socket, message);
        }
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
