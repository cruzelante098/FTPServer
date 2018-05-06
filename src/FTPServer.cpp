//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2ª de grado de Ingeniería Informática
//                       
//                        Main class of the FTP server
// 
//****************************************************************************

#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#include <pthread.h>

#include <list>

#include "Common.h"
#include "FTPServer.h"
#include "ClientConnection.h"

int define_socket_TCP(int port) {

    const MAX_INCOMING_CONNECTIONS = 2;

    int id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (id < 0)
        throw std::system_error(errno, std::system_category(), "socket couldn't be created");

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY); // TODO: comprobar que es la mejor opción (y funciona).

    ssize_t result = bind(id, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
    if (result < 0)
        throw std::system_error(errno, std::system_category(), "bind fail");

    result = listen(id,MAX_INCOMING_CONNECTIONS);
    if(result < 0)
        throw std::system_error(errno, std::system_category(), "listen fail");

    // TODO: Revisar por si está bien.
    return id;
}

// This function is executed when the thread is executed.
void *run_client_connection(void *c) {
    auto connection = (ClientConnection *) c;
    connection->waitForRequests();
    return nullptr;
}


FTPServer::FTPServer(int port) {
    this->port = port;
}

void FTPServer::stop() {
    close(msock);
    shutdown(msock, SHUT_RDWR);
}


// Starting of the server
void FTPServer::run() {
    struct sockaddr_in clientAddress{};
    int clientIdSocket;
    socklen_t alen = sizeof(clientAddress);
    msock = define_socket_TCP(port);  // TODO: This function must be implemented by you.

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        pthread_t thread;
        clientIdSocket = accept(msock, (struct sockaddr *) &clientAddress, &alen);
        if (clientIdSocket < 0)
            errexit("Fallo en el accept: %s\n", strerror(errno)); // TODO: controlar el error de cuando se supera la cola de direcciones.

        auto connection = new ClientConnection(clientIdSocket);

        // Here a thread is created in order to process multiple
        // requests simultaneously. TODO: use C++ STL, maybe?
        pthread_create(&thread, nullptr, run_client_connection, (void *) connection);
    }
#pragma clang diagnostic pop
}
