#include <iostream>
#include <cstdlib>
#include <csignal>

#include "FTPServer.h"

int main(int argc, char** argv) {

	// Signal management
	struct sigaction action{};
	action.sa_sigaction = sighandler;
	action.sa_flags = SA_SIGINFO;

	sigaction(SIGINT, &action, nullptr);    // Ctrl + C
	sigaction(SIGTERM, &action, nullptr);   // Friendly shutdown
	sigaction(SIGHUP, &action, nullptr);    // System shutdown
	sigaction(SIGQUIT, &action, nullptr);   // Close window

	atexit(exitHandler);

	// Run the server!
	server = new FTPServer(2121);
	server->run();
}
