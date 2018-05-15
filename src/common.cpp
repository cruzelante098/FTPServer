#include "common.h"

int createTcpSocket(int mode, uint16_t port, const std::string& ip) {
	int socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_id < 0) return -1;

	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (ip.empty())
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		inet_aton(ip.c_str(), &address.sin_addr);

	ssize_t result = bind(socket_id, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
	if (result < 0) return -1;

	if (mode == LISTEN_MODE)
		if (listen(socket_id, 5) < 0) return -1;

	return socket_id;
}

void* runClientConnection(void* c) {
	// Signal blocking
	sigset_t sigset{};
	sigfillset(&sigset);
	pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

	auto connection = static_cast<ClientConnection*>(c);

	sockaddr_in address{};
	socklen_t len = sizeof address;
	if (getpeername(connection->id(), reinterpret_cast<sockaddr*>(&address), &len) < 0) {
		std::cerr << "getpeername(): " << strerror(errno) << std::endl;
	}

	connection->waitForRequests();

	// Reaching this point means the client connection ended

	--connected_clients;
	std::cout << "[EVENT] Disconnection ";
	std::cout << " - IP: " << inet_ntoa(address.sin_addr);
	std::cout << " - Port: " << ntohs(address.sin_port);
	std::cout << " - Socket ID: " << connection->id();
	std::cout << " <+> Connected: " << connected_clients << std::endl;

	return nullptr; // TODO: Por qué?
}

int connectToClient(uint16_t port, uint32_t address) {
	sockaddr_in client_address{
		.sin_family = AF_INET,
		.sin_port = port,
	};

	client_address.sin_addr.s_addr = address;

	int socket_id = createTcpSocket(CONNECT_MODE, 2020, "127.0.0.1");
	if (socket_id < 0)
		return -1;

	if (connect(socket_id, reinterpret_cast<sockaddr*>(&client_address), sizeof client_address) < 0)
		return -1;
	else
		return socket_id;
}

extern "C" void sighandler(int signum, siginfo_t* info, void* ucontext) {
	std::cout << " -- Signal " << strsignal(signum) << " intercepted" << std::endl;
	server->stop();
	exit(0);
}

void exitHandler() {
	server->stop();
}
