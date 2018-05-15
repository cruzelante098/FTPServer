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
 * Compares @a cmd (string for comparison) with the received @a command (client request)
 */
#define COMMAND(cmd) strcmp(command, cmd)==0

const int MAX_BUFF = 1024; // 1 KB

class ClientConnection {
public:

	/**
	 * Associate @a socket_id with a file for sending responses to client
	 * @param socket_id socket file descriptor
	 */
	explicit ClientConnection(int socket_id);

	/**
	 * Calls to @c stop()
	 */
	~ClientConnection();

	/**
	 * Process clients requests and send responses
	 */
	void waitForRequests();

	/**
	 * Close file and sockets fd's
	 */
	void stop();

	/**
	 * @return @c control_socket
	 */
	int id();

private:

	/**
	 * C file descriptor. We use it to buffer the control connection of the socket and allows to
	 * manage it as a C file usign fprintf, fscanf, etc.
	 */
	FILE *fd;

    const std::string validUsername = "root"; ///< Predefined username

	char command[MAX_BUFF];  ///< Buffer for saving the command.
	char arg[MAX_BUFF];      ///< Buffer for saving the arguments.

	int data_socket;         ///< Data socket descriptor;
	int control_socket;      ///< Control socket descriptor;

	bool ok; 	///< This variable is flag that avoid that the server listens if initialization errors occured.
	bool exit; 	///< Flag to end the waitForRequests() function

	bool logged = false;

private:

	/**************** Command functions ***************/

	int cmdPort();
	int cmdPasv(uint16_t& port, char** ip);
	int cmdList(std::string& data);
	int cmdStor(int file_descriptor);
	int cmdRetr(int file_descriptor);

	/*********** Auxiliary command functions **********/

	/**
	 * Open a file for read only
	 * @return the file descriptor if everything went well, -1 otherwise
	 */
	int openFileForReadOnly(const std::string& name);

	/**
	 * Open a file for read and write
	 * @return the file descriptor if everything went well, -1 otherwise
	 */
	int openFileForReadAndWrite(const std::string& name);

	/**
	 * Send a string to client using data connection
	 * @param data the message
	 * @return sent bytes, -1 if some error occurred
	 */
	ssize_t sendAscii(const std::string& data);

	/**
	 * Check if user is logged in the server
	 * @return true if logged
	 */
	bool isLogged();

	/**
	 * Check if the string constains any '/' (may indicate that the name contains a directory)
	 * @param name
	 * @return true if filename, false if detect a directory in the path
	 */
	bool isAFile(const std::string& name);

	/****************** Other methods *****************/

	/**
	* Print a message error in server's terminal
	* @param error_str describes the error (can be @c strerr(errno) for example)
	* @param function the function name that gave the error
	*/
	void logError(const std::string& error_str, const std::string& function = "unknown");
};

#endif
