#include "ClientConnection.h"
#include "common.h"

int define_socket_TCP(int port) {
	const int MAX_INCOMING_CONNECTIONS = 2; // TODO: comprobar que funciona para dos personas y no para 3

	int id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (id < 0)
		throw std::system_error(errno, std::_V2::system_category(), "ERROR: Socket couldn't be created");

	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_port = htons(static_cast<uint16_t>(port));
	address.sin_addr.s_addr = htonl(INADDR_ANY); // TODO: comprobar que es la mejor opción (y funciona).

	ssize_t result = bind(id, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
	if (result < 0)
		throw std::system_error(errno, std::_V2::system_category(), "bind fail");

	result = listen(id, MAX_INCOMING_CONNECTIONS); // TODO: es necesario hacer el listen?
	if (result < 0)
		// TODO: Es buena idea hacer un throw en esta función?
		throw std::system_error(errno, std::_V2::system_category(), "listen fail");

	return id;
}

void *run_client_connection(void *c) {
	auto connection = (ClientConnection *) c;
	connection->waitForRequests();
	return nullptr;
}

int connectTCP(uint32_t address, uint16_t port) {
	// TODO: Implement your code to define a socket here
	return -1; // TODO: You must return the socket descriptor.

	// TODO: esta función es necesaria?
}
