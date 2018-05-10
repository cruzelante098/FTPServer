#include <iostream>
#include <cstdlib>
#include <csignal>

#include "FTPServer.h"

int main(int argc, char** argv) {
	// Gestión de señales
	struct sigaction action{};
	action.sa_sigaction = sighandler;
	action.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGTERM, &action, nullptr);
	sigaction(SIGHUP, &action, nullptr);
	sigaction(SIGQUIT, &action, nullptr);

	atexit(exit_handler);   // TODO: reejecución de server->stop() por signhandler (?)

	// Creación del servidor
	server = new FTPServer(2121);
	server->run();
}
