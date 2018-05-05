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

#include "common.h"
#include "FTPServer.h"
#include "ClientConnection.h"

int define_socket_TCP(int port)
{
	// TODO: Include the code for defining the socket.
	return -1;
}

// This function is executed when the thread is executed.
void* run_client_connection(void* c)
{
	auto connection = (ClientConnection*) c;
	connection->WaitForRequests();
	return nullptr;
}


FTPServer::FTPServer(int port)
{
	this->port = port;
}


// Parada del servidor.
void FTPServer::stop()
{
	close(msock);
	shutdown(msock, SHUT_RDWR);
}


// Starting of the server
void FTPServer::run()
{
	struct sockaddr_in fsin;
	int ssock;
	socklen_t alen = sizeof(fsin);
	msock = define_socket_TCP(port);  // This function must be implemented by you.

	while (true)
	{
		pthread_t thread;
		ssock = accept(msock, (struct sockaddr*) &fsin, &alen);
		if (ssock < 0)
			errexit("Fallo en el accept: %s\n", strerror(errno));

		auto connection = new ClientConnection(ssock);

		// Here a thread is created in order to process multiple
		// requests simultaneously
		pthread_create(&thread, nullptr, run_client_connection, (void*) connection);
	}
}
