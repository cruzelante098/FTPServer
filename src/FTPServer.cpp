
#include "FTPServer.h"

FTPServer* server;
int connected_clients = 0;

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

	msock = createTcpSocket(LISTEN_MODE, port);
	if (msock < 0)
		throw std::system_error(errno, std::system_category(), strerror(errno));

	// Los pragma son para que el IDE no de advertencias sobre bucle infinito
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"

	while (true) {
		pthread_t thread;
		client_socket = accept(msock, (struct sockaddr*) &client_address, &alen);

		if (client_socket < 0)
			errexit("Fallo en el accept: %s\n", strerror(errno));

		auto connection = new ClientConnection(client_socket);
		connection_list.push_back(connection);

		// Debug log
		++connected_clients;
		std::cout << "[EVENT] New client ";
		std::cout << " - IP: " << inet_ntoa(client_address.sin_addr);
		std::cout << " - Port: " << ntohs(client_address.sin_port);
		std::cout << " - Socket ID: " << client_socket;
		std::cout << " <+> Connected: " << connected_clients << std::endl;

		// Here a thread is created in order to process multiple requests simultaneously.
		pthread_create(&thread, nullptr, runClientConnection, (void*) connection);
		threads.push_back(thread);
	}

	#pragma clang diagnostic pop
}
