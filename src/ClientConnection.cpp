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
	char buffer[MAX_BUFF];

	fd = fdopen(socket_id, "a+");

	if (fd == nullptr) {
		std::cerr << "Connection closed" << std::endl;
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

/**
 * This method processes the requests.
 * Here you should implement the actions related to the FTP commands.
 * See the example for the USER command.
 * If you think that you have to add other commands feel free to do so. You
 * are allowed to add auxiliary methods if necessary.
 */
void ClientConnection::waitForRequests() {
	std::string username;
	bool logged = false;

	if (!ok)
		return;

	fprintf(fd, "220 Service ready.\n");

	while (!exit) {
		std::fill(command, command + MAX_BUFF, 0);
		std::fill(arg, arg + MAX_BUFF, 0);

		fscanf(fd, "%s", command);

		if (COMMAND("USER")) {

			fscanf(fd, "%s", arg);
			username = arg;
			fprintf(fd, "331 User name ok, need password.\n");

		} else if (COMMAND("PWD") || COMMAND("XPWD")) {

			char cwd[200];
			getcwd(cwd, 200);
			fscanf(fd, "%s", arg);
			if (!std::string(arg).empty()) //TODO: comprobar si es correcta
				fprintf(fd, "501 Syntax error in parameters or arguments.\n");
			else
				fprintf(fd, "257 \"%s\" is the current directory.\n", cwd);

		} else if (COMMAND("PASS")) {

			fscanf(fd, "%s", arg);
			if (username == validUsername) {
				if (std::string(arg) == "1234") {
					fprintf(fd, "230 User logged in, proceed.\n");
					logged = true;
				} else {
					fprintf(fd, "530 Not logged in.\n");
				}
			} else {
				fprintf(fd, "332 Need account for login.\n");
			}

		} else if (COMMAND("PORT")) {

			if ((cmdPort()) < 0) {
				fprintf(fd, "501 Syntax error in parameters or arguments.\n");
			} else {
				fprintf(fd, "200 Command okay.\n");
			}

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

		} else if (COMMAND("STOR")) {

			if (logged) {
				fscanf(fd, "%s", arg);

				if (std::string(arg).find('/') != std::string::npos) {
					fprintf(fd, "501 Syntax error in parameters or arguments.\n");
					throw -1;
				}

				char cwd[200];
				getcwd(cwd, 200);
				std::string ruta = std::string(cwd) + '/' + arg;

				int id_archivo = open(arg, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0777);
				if (id_archivo < 0) {
					fprintf(fd, "451 Requested action aborted. Local error in processing.\n");
					throw -1; //TODO: gestionar excepcion
				}

				fprintf(fd, "150 File status okay; about to open data connection.\n");
				fflush(fd);

				char buffer[MAX_BUFF];
				ssize_t bytes = 0;

				while ((bytes = recv(data_socket, buffer, sizeof(buffer), 0)) > 0) {
					// Error in recv()
					if (bytes == -1)
						logError(strerror(errno), "recv()");

					ssize_t result = write(id_archivo, buffer, (size_t) bytes);
					// Error in write()
					if (result == -1)
						logError(strerror(errno), "write()");
					else if (result < bytes) {
						std::stringstream ss;
						ss << "write(): se ha escrito menos cantidad de la especificada: ";
						ss << "escritos " << result << " de " << bytes;
						logError(ss.str(), "write()");
					}
				}

				fprintf(fd, "226 Closing data connection.\n");
				fflush(fd);
				close(id_archivo);
			} else {
				fprintf(fd, "532 Need account for storing files.\n");
			}

		} else if (COMMAND("SYST")) {

			fprintf(fd, "215 UNIX Type: L8.\n");

		} else if (COMMAND("TYPE")) {

			fprintf(fd, "200 Command okay.\n");

		} else if (COMMAND("RETR")) { // Fran

			fscanf(fd, "%s", arg);

			std::string path;
			path.reserve(200);
			getcwd(&path[0], 200);
			path += '/' + arg;

			int file_descriptor = open(arg, O_RDONLY, static_cast<mode_t>(0777));
			if (file_descriptor < 0) {
				fprintf(fd, "550 Requested action not taken.\n");
				logError(strerror(errno), "open()");
			} else {
				fprintf(fd, "150"); // TODO:
				char buffer[1024];
				ssize_t bytes = 0;

				while ((bytes = read(file_descriptor, buffer, 1024))) {
					send(data_socket, buffer, (size_t) bytes, 0); // TODO: gestionar errores
				}

				fprintf(fd, "226"); // TODO:
				close(file_descriptor);
			}

		} else if (COMMAND("QUIT")) {

			fprintf(fd, "221 Service closing control connection. Goodbye.\n");
			stop();

		} else if (COMMAND("LIST")) {

			std::string ls;

			if (cmdList(ls) < 0)
				fprintf(fd, "450 Requested file action not taken.\n");
			else {
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
	std::cerr << " -- Client " << id();
	std::cerr << " -- in function " << function;
	std::cerr << " -- REASON: " << error_str;
	std::cerr << " -- Received command: " << command << " " << arg;
	std::cerr << std::endl;
}

int ClientConnection::cmdPort() {
	if (data_socket != -1) close(data_socket);

	std::array<uint32_t, 4> ip{};
	std::array<uint16_t, 2> port{};
	fscanf(fd, "%d,%d,%d,%d,%hi,%hi", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

	uint32_t ip_bin = ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0];
	uint16_t port_bin = port[1] << 8 | port[0];

	data_socket = connectToClient(port_bin, ip_bin);
	if (data_socket < 0)
		logError(strerror(errno), "TODO");
	return data_socket;
}

int ClientConnection::cmdPasv(uint16_t& port, char** ip) {
	if (data_socket != -1) close(data_socket);
	int sockid = createTcpSocket(LISTEN_MODE, 0, "127.0.0.1");

	if (sockid < 0)
		return -1;

	sockaddr_in address{};
	socklen_t len = sizeof(address);

	// used for the message of entering passive mode
	if (getsockname(sockid, reinterpret_cast<sockaddr*>(&address), &len) < 0) {
		logError(strerror(errno), "getsockname()");
		return -1;
	}

	port = address.sin_port;

	char* ip_bin = inet_ntoa(address.sin_addr);
	ip[0] = strtok(ip_bin, ".");
	ip[1] = strtok(nullptr, ".");
	ip[2] = strtok(nullptr, ".");
	ip[3] = strtok(nullptr, ".");

	if ((data_socket = accept(sockid, reinterpret_cast<sockaddr*>(&address), &len)) < 0) {
		logError(strerror(errno), "accept()");
		return -1;
	}
}

int ClientConnection::cmdList(std::string& buffer) {
	std::array<int, 2> pipefd{};

	if (pipe(pipefd.data()) != 0) {
		logError(strerror(errno), "pipe()");
		return -1;
	}

	std::array<char, 9000> buff{};
	const int& child_output = pipefd[0];
	const int& parent_input = pipefd[1];

	pid_t pid = fork();

	if (pid == 0) {

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

		close(parent_input);
		read(child_output, buff.data(), buff.size()); // TODO: Control de errores
		wait(nullptr);

	} else {

		logError(strerror(errno), "fork()");
		return -1;

	}

	buffer = buff.data(); // TODO: solucionar porque no se está copiando
	return 0;
}

ssize_t ClientConnection::sendAscii(const std::string& str) {
	ssize_t bytes = send(data_socket, str.c_str(), str.size(), 0); // TODO: gestionar errores
	if (bytes < 0)
		logError(strerror(errno), "send()");
	return bytes;
}
