#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <cerrno>
#include <cstring>
#include <cstdio>

#include <list>
#include <vector>
#include <iomanip>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "ClientConnection.h"

class ClientConnection;

class FTPServer {
public:
	explicit FTPServer(uint16_t port = 2121);
	void run();
	void stop();

private:
	uint16_t port;
	int msock;
	std::list<ClientConnection*> connection_list;
	std::list<pthread_t> threads;
};

extern FTPServer* server;
extern int clients;

#endif
