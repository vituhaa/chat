#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
using namespace std;

int main()
{
    // cоздание сокета
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        cerr << "ERROR: cannot create socket" << endl;
        return 1;
    }
    // установка адреса и порта сервера
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);                    // порт 12345
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // адрес сервера (localhost)

    // установка соединения с сервером
    if (connect(client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << "ERROR: cannot connect to server" << endl;
        close(client_socket);
        return 1;
    }
    // отправка сообщения серверу
    const char *message = "Hello, server!";
    send(client_socket, message, strlen(message), 0);

    // Получение ответа от сервера
    char buffer[1024];
    int bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytesReceived == -1)
    {
        cerr << "ERROR: cannot receive data from server" << endl;
    }
    else
    {
        buffer[bytesReceived] = '\0';
        cout << "Received response from server: " << buffer << endl;
    }

    // закрытие соединения
    close(client_socket);
    return 0;
}
