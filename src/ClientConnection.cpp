//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2ª de grado de Ingeniería Informática
//                       
//              This class processes an FTP transactions.
// 
//****************************************************************************

#include "ClientConnection.h"

ClientConnection::ClientConnection(int socket_id) {
	control_socket = socket_id;

	fd = fdopen(socket_id, "a+");

	if (fd == nullptr) {
		std::cerr << "[ERROR] Connection closed: can't associate the socket file descriptor with a file" << std::endl;
		fclose(fd);
		close(control_socket);
		ok = false;
		return;
	}

	ok = true;
	data_socket = -1;
}


ClientConnection::~ClientConnection() {
	this->stop();
}


void ClientConnection::stop() {
	fclose(fd);
	close(data_socket);
	close(control_socket);
	exit = true;
}

void ClientConnection::waitForRequests() {
	std::string username;

	if (!ok)
		return;

	fprintf(fd, "220 Service ready.\n");
	fflush(fd);

	while (!exit) {
		std::fill(command, command + MAX_BUFF, 0);
		std::fill(arg, arg + MAX_BUFF, 0);

		fscanf(fd, "%s", command);

		if (COMMAND("USER")) {
			fscanf(fd, "%s", arg);
			username = arg;
			fprintf(fd, "331 User name ok, need password.\n");
			fflush(fd);
		} else if (COMMAND("PWD") || COMMAND("XPWD")) {
			char cwd[200];
			getcwd(cwd, 200);
			fprintf(fd, "257 \"%s\" is the current directory.\n", cwd);
			fflush(fd);
		} else if (COMMAND("PASS")) {
			fscanf(fd, "%s", arg);
			if (username == validUsername && std::string(arg) == "1234") {
				fprintf(fd, "230 User logged in, proceed.\n");
				logged = true;
			} else {
				fprintf(fd, "530 Not logged in.\n");
			}
			fflush(fd);
		} else if (COMMAND("PORT")) {
			if ((cmdPort()) < 0) {
				fprintf(fd, "501 Syntax error in parameters or arguments.\n");
			} else {
				fprintf(fd, "200 Command okay.\n");
			}
			fflush(fd);
		} else if (COMMAND("PASV")) {
			char** ip = nullptr;
			uint16_t port;
			if (cmdPasv(port, ip) < 0)
				fprintf(fd, "550 Requested action not taken.\n");
			else
				fprintf(fd, "227 Entering Passive Mode (%s,%s,%s,%s,%i,%i)\n",
				        ip[0], ip[1], ip[2], ip[3], port % 256, port / 256);
			fflush(fd);
		} else if (COMMAND("CWD")) {
			fprintf(fd, "250 Requested file action okay, completed.\n");
			fflush(fd);
		} else if (COMMAND("STOR")) {
			if (!isLogged()) {
				fprintf(fd, "532 Need account for storing files.\n");
				fflush(fd);
			} else {
				fscanf(fd, "%s", arg);
				if (!isThatAFile(arg)) {
					fprintf(fd, "501 Syntax error in parameters or arguments.\n");
					fflush(fd);
				} else {
					int stor_file_descriptor = openFileForReadAndWrite(arg);
					if (stor_file_descriptor < 0) {
						fprintf(fd, "451 Requested action aborted. Local error in processing.\n");
						fflush(fd);
					} else {
						fprintf(fd, "150 File status okay; about to open data connection.\n");
						fflush(fd);
						if (cmdStor(stor_file_descriptor) < 0)
							fprintf(fd, "451 Requested action aborted. Local error in processing.\n");
						else
							fprintf(fd, "226 Closing data connection.\n");
						fflush(fd);
					}
				}
			}
		} else if (COMMAND("SYST")) {
			fprintf(fd, "215 UNIX Type: L8.\n");
			fflush(fd);
		} else if (COMMAND("TYPE")) {
			fprintf(fd, "200 Command okay.\n");
			fflush(fd);
		} else if (COMMAND("RETR")) {
			if (!isLogged()) {
				fprintf(fd, "532 Need account for storing files.\n");
				fflush(fd);
			} else {
				fscanf(fd, "%s", arg);
				int file_descriptor = openFileForReadOnly(arg);
				if (file_descriptor < 0) {
					fprintf(fd, "450 Requested file action not taken. File unavailable\n");
					fflush(fd);
				} else {
					fprintf(fd, "150 File status okay; about to open data connection.\n");
					fflush(fd);
					if (cmdRetr(file_descriptor) < 0) {
						fprintf(fd, "426 Connection closed; transfer aborted.\n");
						fflush(fd);
					} else {
						fprintf(fd, "226 Closing data connection. Requested file action successful.\n");
						fflush(fd);
					}
				}
			}
		} else if (COMMAND("QUIT")) {
			fprintf(fd, "221 Service closing control connection. Goodbye.\n");
			fflush(fd);
			stop();
		} else if (COMMAND("LIST")) {
			std::string ls;
			if (cmdList(ls) < 0) {
				fprintf(fd, "450 Requested file action not taken.\n");
				fflush(fd);
			} else {
				fprintf(fd, "150 File status okay; about to open data connection.\n");
				fflush(fd);
				if (sendAscii(ls) < 0) {
					fprintf(fd, "451 Requested action aborted: local error in processing.\n");
				} else
					fprintf(fd, "226 Transfer successful.\n");
				fflush(fd);
			}
		} else {
			fprintf(fd, "502 Command not implemented.\n");
			fflush(fd);
			logError("Command not recognized");
		}
	}
	fclose(fd);
}

int ClientConnection::id() {
	return control_socket;
}

void ClientConnection::logError(const std::string& error_str, const std::string& function) {
	std::cerr << "[ERROR]";
	std::cerr << " From client " << id();
	std::cerr << " in function `" << function << "'";
	std::cerr << " - Reason: " << error_str;
	std::cerr << " - Received command: " << command << " " << arg;
	std::cerr << std::endl;
}

int ClientConnection::cmdPort() {
	if (data_socket != -1)
		close(data_socket);

	std::array<uint32_t, 4> ip{};
	std::array<uint16_t, 2> port{};

	fscanf(fd, "%d,%d,%d,%d,%hi,%hi", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]); // NOLINT

	uint32_t ip_bin = ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0]; // NOLINT
	uint16_t port_bin = port[1] << 8 | port[0]; // NOLINT

	data_socket = connectToClient(port_bin, ip_bin);

	if (data_socket < 0)
		logError(strerror(errno), "cmdPort()");

	return data_socket;
}

int ClientConnection::cmdPasv(uint16_t& port, char** ip) {
	if (data_socket != -1)
		close(data_socket);

	int sockid = createTcpSocket(LISTEN_MODE, 0, "127.0.0.1");

	if (sockid < 0)
		return -1;

	sockaddr_in address{};
	socklen_t len = sizeof(address);

	// Used for the message of entering passive mode
	if (getsockname(sockid, reinterpret_cast<sockaddr*>(&address), &len) < 0) {
		logError(strerror(errno), "getsockname()");
		return -1;
	}

	port = address.sin_port;
	char* ip_bin = inet_ntoa(address.sin_addr);

	// Tokenization of ip, spliting by '.'
	ip[0] = strtok(ip_bin, ".");
	ip[1] = strtok(nullptr, ".");
	ip[2] = strtok(nullptr, ".");
	ip[3] = strtok(nullptr, ".");

	// Accepts user's connection
	if ((data_socket = accept(sockid, reinterpret_cast<sockaddr*>(&address), &len)) < 0) {
		logError(strerror(errno), "accept()");
		return -1;
	}
}

int ClientConnection::cmdList(std::string& data) {
	std::array<int, 2> pipefd{};

	if (pipe(pipefd.data()) != 0) {
		logError(strerror(errno), "pipe()");
		return -1;
	}

	std::array<char, 9000> buffer{};
	const int& child_output = pipefd[0];
	const int& parent_input = pipefd[1];

	pid_t pid = fork();


	if (pid == 0) {

		// Son
		close(child_output);

		if (dup2(parent_input, STDOUT_FILENO) < 0 || dup2(parent_input, STDERR_FILENO) < 0) {
			logError(strerror(errno), "dup2()");
			return -1;
		}

		if (execlp("bash", "bash", "-c", "ls -l", nullptr) == -1) {
			logError(strerror(errno), "execlp()");
			return -1;
		}

	} else if (pid > 0) {

		// Father
		close(parent_input);
		if (read(child_output, buffer.data(), buffer.size()) < 0) {
			logError(strerror(errno), "read()");
			return -1;
		}
		wait(nullptr);

	} else {
		logError(strerror(errno), "fork()");
		return -1;
	}

	data = buffer.data();
	return 0;
}

int ClientConnection::cmdRetr(int file_descriptor) {
	char buffer[MAX_BUFF];
	ssize_t read_bytes = 0;

	while ((read_bytes = read(file_descriptor, buffer, MAX_BUFF))) {

		if (read_bytes < 0) {
			logError(strerror(errno), "read()");
			return -1;
		}

		ssize_t sent_bytes = send(data_socket, buffer, (size_t) read_bytes, 0);

		if (sent_bytes == -1) {
			logError(strerror(errno), "send()");
			return -1;
		} else if (sent_bytes < read_bytes) {
			std::stringstream ss;
			ss << "Se ha enviado menos cantidad de la especificada: ";
			ss << "enviados " << sent_bytes << " de " << read_bytes;
			logError(ss.str(), "send()");
			return -1;
		}
	}

	close(file_descriptor);
	close(data_socket);

	return 0;
}

int ClientConnection::cmdStor(int file_descriptor) {
	char buffer[MAX_BUFF];
	ssize_t bytes = 0;
	while ((bytes = recv(data_socket, buffer, sizeof(buffer), 0)) > 0) {

		// Error in recv()
		if (bytes == -1) {
			logError(strerror(errno), "recv()");
			return -1;
		}

		ssize_t result = write(file_descriptor, buffer, (size_t) bytes);

		// Error in write()
		if (result == -1) {
			logError(strerror(errno), "write()");
			return -1;
		} else if (result < bytes) {
			std::stringstream ss;
			ss << "write(): se ha escrito menos cantidad de la especificada: ";
			ss << "escritos " << result << " de " << bytes;
			logError(ss.str(), "write()");
			return -1;
		}

	}
	close(file_descriptor);
	close(data_socket);
	return 0;
}

ssize_t ClientConnection::sendAscii(const std::string& data) {

	ssize_t sent_bytes = send(data_socket, data.c_str(), data.size(), 0);

	if (sent_bytes == -1) {
		logError(strerror(errno), "send()");
		return -1;
	} else if (sent_bytes < data.size()) {
		std::stringstream ss;
		ss << "Se ha enviado menos cantidad de la especificada: ";
		ss << "enviados " << sent_bytes << " de " << data.size();
		logError(ss.str(), "send()");
		return -1;
	}

	close(data_socket);
	return sent_bytes;
}

int ClientConnection::openFileForReadOnly(const std::string& name) {
	char cwd[200];
	getcwd(cwd, 200);

	std::string path = std::string(cwd) + '/' + name;

	int file_descriptor = open(path.c_str(), O_RDONLY, static_cast<mode_t>(0777));
	if (file_descriptor < 0) {
		logError(strerror(errno), "open()");
		return -1;
	}

	return file_descriptor;
}

bool ClientConnection::isLogged() {
	return logged;
}

bool ClientConnection::isThatAFile(const std::string& name) {
	return std::string(arg).find('/') == std::string::npos;
}

int ClientConnection::openFileForReadAndWrite(const std::string& name) {
	char cwd[200];
	getcwd(cwd, 200);
	std::string ruta = std::string(cwd) + '/' + name;

	int id_archivo = open(ruta.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0777);
	if (id_archivo < 0) {
		logError(strerror(errno), "open()");
		return -1;
	}
	return id_archivo;
}
