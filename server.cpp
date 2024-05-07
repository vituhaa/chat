#include <iostream>
#include <cstring>
#include <WinSock2.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <map>
#include <queue>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

mutex clients_mutex;
vector<SOCKET> clients;
unordered_map<string, string> users; // Хранит имя пользователя и пароль
unordered_map<string, SOCKET> user_sockets; // Соответствие между именем пользователя и его сокетом
map<pair<string, string>, queue<string>> message_history; // История сообщений между пользователями

bool loadUsers() {
    ifstream file("users.txt");
    if (!file.is_open()) return false;
    string line, username, password;
    while (getline(file, line)) {
        stringstream ss(line);
        ss >> username >> password;
        users[username] = password;
    }
    file.close();
    return true;
}

bool registerUser(const string& username, const string& password) {
    if (users.find(username) != users.end()) return false; // Пользователь уже существует
    ofstream file("users.txt", ios::app);
    if (!file.is_open()) return false;
    file << username << " " << password << endl;
    users[username] = password;
    file.close();
    return true;
}

bool authenticateUser(const string& username, const string& password) {
    auto it = users.find(username);
    if (it == users.end()) return false; // Пользователь не найден
    return it->second == password;
}

void sendUserList(SOCKET client_socket) {
    string userList = "List of users:\n";
    for (auto& user : users) {
        userList += user.first + "\n";
    }
    send(client_socket, userList.c_str(), userList.size(), 0);
}

void sendMessageToUser(const string& sender, const string& receiver, const string& message) {
    if (user_sockets.count(receiver) > 0) {
        send(user_sockets[receiver], message.c_str(), message.size(), 0);
    }
    message_history[{sender, receiver}].push(message);
}

void clientHandler(SOCKET client_socket) {
    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    buffer[bytes_received] = '\0';
    string cmd, username, password;
    stringstream ss(buffer);
    ss >> cmd >> username >> password;

    if (cmd == "register") {
        if (!registerUser(username, password)) {
            const char* response = "Registration failed. User already exists.";
            send(client_socket, response, strlen(response), 0);
            closesocket(client_socket);
            return;
        }
    } else if (cmd == "login") {
        if (!authenticateUser(username, password)) {
            const char* response = "Login failed. Incorrect username or password.";
            send(client_socket, response, strlen(response), 0);
            closesocket(client_socket);
            return;
        }
    }

    const char* success = "success";
    send(client_socket, success, strlen(success), 0);
    user_sockets[username] = client_socket; // Store user socket

    cout << username << " joined the chat." << endl;
    
    clients_mutex.lock();
    clients.push_back(client_socket);
    clients_mutex.unlock();

    // Handling additional commands
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break; // Соединение прервано или ошибка
        }

        buffer[bytes_received] = '\0';
        string cmd, recipient;
        stringstream ss(buffer);
        ss >> cmd;
        if (cmd == "get_users") {
            sendUserList(client_socket);
        } else if (cmd == "send_msg") {
            ss >> recipient;
            string message = ss.str().substr(ss.tellg());
            sendMessageToUser(username, recipient, message);
        }
    }


    cout << username << " left the chat." << endl;
    clients_mutex.lock();
    clients.erase(remove(clients.begin(), clients.end(), client_socket), clients.end());
    user_sockets.erase(username); // Remove user socket
    clients_mutex.unlock();
    closesocket(client_socket);
}

int main() {
    loadUsers();

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    listen(server_socket, 5);

    cout << "Server started..." << endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            continue; // Продолжить прием соединений, если accept не удался
        }
        thread(clientHandler, client_socket).detach(); // Обработать клиента в отдельном потоке
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
