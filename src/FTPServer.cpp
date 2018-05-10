#include <vector>
#include <iomanip>
#include "FTPServer.h"

FTPServer* server;
int clients = 0;

FTPServer::FTPServer(uint16_t port) : port(port) {}

void FTPServer::stop() {
	close(msock);
	shutdown(msock, SHUT_RDWR);

	// Close clients connection threads
	for (auto&& thread : threads) {
		pthread_cancel(thread);
		pthread_detach(thread);
	}
}


void FTPServer::run() {
	struct sockaddr_in client_address{};
	int client_socket;
	socklen_t alen = sizeof(client_address);

	msock = define_socket_TCP(port);
	if (msock < 0)
		throw std::system_error(errno, std::system_category(), strerror(errno));

	// Los pragma son para que el IDE no de advertencias sobre bucle infinito
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"

	while (true) {
		pthread_t thread;
		client_socket = accept(msock, (struct sockaddr*) &client_address, &alen);

		// TODO: controlar el error de cuando se supera la cola de direcciones.
		if (client_socket < 0)
			errexit("Fallo en el accept: %s\n", strerror(errno)); // TODO: Por qué usar errexit?

		auto connection = new ClientConnection(client_socket);
		connection_list.push_back(connection);

		// Debug log
		++clients;
		std::cout << "Nuevo cliente";
		std::cout << "   IP: " << inet_ntoa(client_address.sin_addr);
		std::cout << "   Puerto: " << ntohs(client_address.sin_port);
		std::cout << "   ID: " << client_socket << std::endl;
		std::cout << "Conectados: " << clients << std::endl;

		// Here a thread is created in order to process multiple requests simultaneously.
		pthread_create(&thread, nullptr, run_client_connection, (void*) connection);
		threads.push_back(thread);
	}

	#pragma clang diagnostic pop
}
