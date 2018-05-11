#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <pthread.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <err.h>
#include <sstream>

#include <netdb.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pwd.h>
#include <grp.h>
#include <langinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include <ctime>
#include <clocale>

#include <sys/stat.h>
#include <iostream>
#include <dirent.h>

#include "common.h"

/**
 * Compares @a cmd with the received @a command
 */
#define COMMAND(cmd) strcmp(command, cmd)==0

const int MAX_BUFF = 1000;

class ClientConnection {
public:
	explicit ClientConnection(int socket_id);
	~ClientConnection();
	void waitForRequests();
	void stop();
	int id();

private:

	/**
	 * C file descriptor. We use it to buffer the control connection of the socket and allows to
	 * manage it as a C file usign fprintf, fscanf, etc.
	 */
	FILE *fd;

    const std::string validUsername = "root";

	char command[MAX_BUFF];  ///< Buffer for saving the command.
	char arg[MAX_BUFF];      ///< Buffer for saving the arguments.

	int data_socket;         ///< Data socket descriptor;
	int control_socket;      ///< Control socket descriptor;

	bool ok; ///< This variable is flag that avoid that the server listens if initialization errors occured.
	bool exit; ///< Flag to end the waitForRequests() function

private:
	void logError(const std::string& error_str, const std::string& function = "unknown");
	int cmdPort();
	int cmdPasv(uint16_t& port, char** ip);
	int cmdList(std::string& buffer);

	ssize_t sendAscii(const std::string& str);
};

#endif
