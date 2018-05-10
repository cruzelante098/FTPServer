#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <iostream>
#include <signal.h>

#include "FTPServer.h"
#include "ClientConnection.h"

inline void errexit(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

int connectTCP(uint32_t address, uint16_t port);
int define_socket_TCP(uint16_t port, const std::string& ip = "");
void* run_client_connection(void* c);
extern "C" void sighandler(int signum, siginfo_t* info, void* ucontext);
void exit_handler();

#endif
