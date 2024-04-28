#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
// три библиотеки сверху на винде нужно будет заменить на одну (погугли)
using namespace std;

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - использование сетевого протокола IPv4
                                                         //(протокол для передачи информации внутри сети Интернет. )
                                                         // SOCKET_STREAM - использование TCP (Протокол управления передачей)
                                                         // для потоковой передачи данных
    if (server_socket == -1)
    {
        cerr << "ERROR: cannot create socket" << endl;
        return 1;
    }

    struct sockaddr_in serverAddr;           // стурктура sockaddr_int, которая содержит информацию о сетевом адресе и порте сервера
    serverAddr.sin_family = AF_INET;         // это поле структуры sockaddr_in,которое указывает на семейство протоколов адресов
    serverAddr.sin_port = htons(12345);      // Порт 12345. поле стурктуры,которое содержит номер порта сервера. значение порта представлено в сетевом порядке байтов
    serverAddr.sin_addr.s_addr = INADDR_ANY; // sin_addr.s_addr - поле, которое содержит айпи-адрес сервера. sin_addr - структура in_addr.
                                             // s_addr - член этой структуры,представляющий 32-битное значение айпи-адреса в сетевом порядке байтов.
                                             // INADDR_ANY - сервер будет принимать соединения на всех доступных сетевых интерфейсах
    if (bind(server_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        // bind() - функция,которая привязывает сокет к указанному адресу и порту на локальной машине. если привязка не удалась,программа выводит соотв. сообщение
        // и завершает выполнение.
        // про функцию bind(): необходима для того,чтобы ОС знала,к какому адресу и порту относится данный сокет,когда он используется для обмена данными.
        // 1-й параметр - указывает на сокет,который нужно привязать к определённому адресу и порту.
        // 2-й параметр - указатель на структуру,которая представляет собой адрес ,к которому мы хотим привязать сокет.
        // 3-й параметр - размер структуры,на которую указывает addr.
        // если привязка успешная - вернёт 0.
        cerr << "ERROR: cannot bind socket to address" << endl;
        close(server_socket);
        return 1;
    }
    // установка сокета в режим прослушивания входящих соединений
    if (listen(server_socket, 5) == -1)
    {
        // 5 - макс. кол-во ожидающих соединений в очереди.
        cerr << "ERROR: cannot listen on socket" << endl;
        close(server_socket);
        return 1;
    }
    int client_socket = accept(server_socket, NULL, NULL); // принятие входящего соединения от клиента с помощью ф-ии accept(). функция блокируется,
                                                           // пока не появится входящее сообщение.
    //что accept() принимает на вход:
    //1-й параметр - дескриптор сокета,который был создан и привязан к адресу с помощью socket() и bind(). этот сокет должен быть в режиме
    //прослушивания входящих соединений с помощью функции listen()
    //2-й параметр - указатель на структуру,в которую будет сохранён адрес клиента,который подключается к серверу.
    //3-й параметр - указатель на переменную типа socklen_t,которая содержит размер структуры,на которую указывает addr. после успешного вызова
    //accept() этот параметр будет содержать размер стурктуры, в который был записан адрес клиента.
    if (client_socket == -1)
    {
        cerr<<"ERROR: cannot accept incoming connection"<<endl;
        close(server_socket);
        return 1;
    }

    //получение сообщения от клиента:
    char buffer[1024];
    //получение данных от клиента с посощью функции recv(). buffer - буфер для сохранения принятых данных.
    int bytes_received = recv(client_socket, buffer, sizeof(buffer),0);
    //1-й параметр - дескриптор сокета,из которого мы хотим получить данные
    //2-й параметр - указатель на буфер,в который будут сохранены принятые данные
    //3-й параметр - размер буфера, recv() не примет больше данных,чем размер буфера
    //4-й параметр - флаг,который может влиять на поведение операции приёма данных (в нашем случае 0 - для стандартного поведения)
    if (bytes_received == -1)
    {
        cerr<<"ERROR: cannot receive data from client"<<endl;
    }
    else
    {
        buffer[bytes_received]='\0';
        cout<<"Received message from client: "<<buffer<<endl;
        //вывод принятого сообщения от клиента
    }

    //отправка ответа клиенту:
    const char* response = "Message received!";
    send(client_socket, response, strlen(response),0);
    // закрытие соединений
    close(client_socket);
    close(server_socket);
    return 0;
}