#include "common.h"

int define_socket_TCP(uint16_t port, const std::string &ip) {
    const int MAX_INCOMING_CONNECTIONS = 2; // TODO: comprobar que funciona para dos personas y no para 3

    int id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (id < 0)
        return -1;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (ip.empty())
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_aton(ip.c_str(), &address.sin_addr);

    ssize_t result = bind(id, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
    if (result < 0)
        return -1;

    result = listen(id, MAX_INCOMING_CONNECTIONS); // TODO: es necesario hacer el listen?
    if (result < 0)
        return -1;

    return id;
}

int connect_socket_TCP(uint16_t port, const std::string &ip) {
    const int MAX_INCOMING_CONNECTIONS = 2; // TODO: comprobar que funciona para dos personas y no para 3

    int id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (id < 0)
        return -1;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (ip.empty())
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_aton(ip.c_str(), &address.sin_addr);

    ssize_t result = bind(id, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
    if (result < 0)
        return -1;

    return id;
}

void *run_client_connection(void *c) {
    // Signal blocking
    sigset_t sigset{};
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    auto connection = static_cast<ClientConnection *>(c);
    connection->waitForRequests();

    --clients;
    std::cout << "Cliente " << connection->id() << " desconectado. Conectados: " << clients << std::endl;

    return nullptr;
}

int connectTCP(uint32_t address, uint16_t port) {
    sockaddr_in client_address{
            .sin_family = AF_INET,
            .sin_port = port,
    };

    client_address.sin_addr.s_addr = address;

    int socket_id = connect_socket_TCP(2020, "127.0.0.1");
    if (socket_id < 0) return -1;

    return connect(socket_id, reinterpret_cast<sockaddr *>(&client_address), sizeof client_address);
}

extern "C" void sighandler(int signum, siginfo_t *info, void *ucontext) {
    std::cout << " -- Signal " << strsignal(signum) << " intercepted" << std::endl;
    server->stop();
    exit(0);
}

void exit_handler() {
    server->stop();
}
