#include <vector>
#include "FTPServer.h"

FTPServer* server;
int clients = 0;

FTPServer::FTPServer(uint16_t port) : port(port) {}

void FTPServer::stop() {
	close(msock);
	shutdown(msock, SHUT_RDWR);

	// Close clients connection threads
	for(auto&& thread : threads)
	{
		pthread_cancel(thread);
		pthread_detach(thread);
	}
}


void FTPServer::run() {
	struct sockaddr_in clientAddress{};
	int clientIdSocket;
	socklen_t alen = sizeof(clientAddress);

	// TODO: Mejorar la gestión de errores
	try {
		msock = define_socket_TCP(port);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		throw e;
	}

	// Los pragma son para que el IDE no de advertencias sobre bucle infinito
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"

	while (true) {
		pthread_t thread; // TODO: guardar los hilos para cancelación posterior!!!!!!
		clientIdSocket = accept(msock, (struct sockaddr*) &clientAddress, &alen);

		// TODO: controlar el error de cuando se supera la cola de direcciones.
		if (clientIdSocket < 0)
			errexit("Fallo en el accept: %s\n", strerror(errno));

		auto connection = new ClientConnection(clientIdSocket);
		connection_list.push_back(connection);

		// Debug log
		++clients;
		std::cout << "Cliente: " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " -- ";
		std::cout << "ID: " << clientIdSocket << std::endl;
		std::cout << "Conectados: " << clients << std::endl;

		// Here a thread is created in order to process multiple requests simultaneously.
		pthread_create(&thread, nullptr, run_client_connection, (void*) connection);
		threads.push_back(thread);
	}

	#pragma clang diagnostic pop
}
