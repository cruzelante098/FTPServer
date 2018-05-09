#include "FTPServer.h"

FTPServer::FTPServer(int port) {
	this->port = port;
}

void FTPServer::stop() {
	close(msock);
	shutdown(msock, SHUT_RDWR);
}


void FTPServer::run() {
	struct sockaddr_in clientAddress{};
	int clientIdSocket;
	socklen_t alen = sizeof(clientAddress);

	// TODO: Mejorar la gestión de errores
	try {
		msock = define_socket_TCP(port);
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		throw e;
	}

	// Los pragma son para que el IDE no de advertencias sobre bucle infinito
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"

	while (true) {
		pthread_t thread; // TODO: no sería mejor usar la STL de C++?
		clientIdSocket = accept(msock, (struct sockaddr *) &clientAddress, &alen);

		// TODO: controlar el error de cuando se supera la cola de direcciones.
		if (clientIdSocket < 0)
			errexit("Fallo en el accept: %s\n", strerror(errno));

		auto connection = new ClientConnection(clientIdSocket);

		// Here a thread is created in order to process multiple requests simultaneously.
		pthread_create(&thread, nullptr, run_client_connection, (void *) connection);
	}

	#pragma clang diagnostic pop
}
