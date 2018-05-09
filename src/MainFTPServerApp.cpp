#include <iostream>
#include <csignal>

#include "FTPServer.h"

FTPServer *server;

extern "C" void sighandler(int signal, siginfo_t *info, void *ptr) {
	std::cout << "Dispara sigaction" << std::endl;
	server->stop();
	exit(-1);
}

void exit_handler() {
	server->stop();
}


int main(int argc, char **argv) {
	struct sigaction action;
	action.sa_sigaction = sighandler;
	action.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &action, nullptr);
	server = new FTPServer();
	atexit(exit_handler);   // TODO: reejecución de server->stop() por signhandler (?)
	server->run();
}
