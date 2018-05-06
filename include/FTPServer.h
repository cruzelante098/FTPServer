#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <cerrno>
#include <cstring>
#include <cstdio>

#include <list>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "ClientConnection.h"

class FTPServer {
public:
	explicit FTPServer(int port = 2121);

	void run();

	void stop();

private:
	int port;
	int msock;
	std::list<ClientConnection *> connection_list;
};

#endif
