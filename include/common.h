#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <iostream>
#include <signal.h>

#include "FTPServer.h"
#include "ClientConnection.h"

/**
 * Used for createSocketTCP()
 */
enum {
	LISTEN_MODE,
	CONNECT_MODE
};

inline void errexit(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

int connectToClient(uint16_t port, uint32_t address);
int createTcpSocket(int mode, uint16_t port, const std::string& ip = "127.0.0.1");
void* runClientConnection(void* c);
extern "C" void sighandler(int signum, siginfo_t* info, void* ucontext);
void exitHandler();

#endif
