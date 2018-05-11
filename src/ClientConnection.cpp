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
	if (!ok)
		return;
	fprintf(fd, "220 Service ready.\n");
	while (!exit) {
		fscanf(fd, "%s", command);
		if (COMMAND("USER")) {

			fscanf(fd, "%s", arg);
			fprintf(fd, "331 User name ok, need password.\n");

		} else if (COMMAND("PWD") || COMMAND("XPWD")) {

			char cwd[200];
			getcwd(cwd, 200);
			fprintf(fd, "257 \"%s\" is the current directory.\n", cwd);

		} else if (COMMAND("PASS")) {

			fscanf(fd, "%s", arg);
			fprintf(fd, "230 User logged in, proceed.\n");

		} else if (COMMAND("PORT")) {

			if (data_socket < 0) {
				fprintf(fd, "550 Requested action not taken.\n");
				logError(strerror(errno));
			} else {
				fprintf(fd, "200 Command okay.\n");
			}

		} else if (COMMAND("PASV")) { // Jorge

			char** ip = nullptr;
			uint16_t port;

			if (cmdPasv(port, ip) < 0)
				fprintf(fd, "500 Syntax error, command unrecognized.\n");
			else
				fprintf(fd, "227 Entering Passive Mode (%s,%s,%s,%s,%i,%i)\n",
				        ip[0], ip[1], ip[2], ip[3], port % 256, port / 256);
			fflush(fd);

		} else if (COMMAND("CWD")) {

			// TODO: implementar de verdad ?
			fprintf(fd, "250 Requested file action okay, completed.\n");

		} else if (COMMAND("STOR")) { // Jorge

			fscanf(fd, "%s", arg);
			char cwd[200];
			getcwd(cwd, 200);
			std::string ruta = std::string(cwd) + '/' + arg;

			int id_archivo = open(arg, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0777);
			if (id_archivo < 0)
				throw -1; //TODO: gestionar excepcion


			fprintf(fd, "150 File status okay; about to open data connection.\n");
			fflush(fd);

			char buffer[MAX_BUFF];
			ssize_t bytes = 0;

			while ((bytes = recv(data_socket, buffer, sizeof(buffer), 0)) > 0) {
				// Error in recv()
				if (bytes == -1)
					logError(std::string("recv(): ") + strerror(errno));

				ssize_t result = write(id_archivo, buffer, (size_t) bytes);
				// Error in write()
				if (result == -1)
					logError(strerror(errno));
				else if (result < bytes) {
					std::stringstream ss;
					ss << "write(): se ha escrito menos cantidad de la especificada: ";
					ss << "escritos " << result << " de " << bytes;
					logError(ss.str());
				}
			}

			fprintf(fd, "226 Closing data connection.\n");
			fflush(fd);
			close(id_archivo);

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
				logError(strerror(errno));
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

			std::array<int, 2> pipefd{};
			if (pipe(pipefd.data()) != 0)
				throw std::system_error(errno, std::system_category(), "pipe failed");

			std::array<char, 9000> buffer{};
			const int& child_output = pipefd[0];
			const int& parent_input = pipefd[1];

			pid_t pid = fork();
			// TODO: corregir código (para que se envie a través de send) y excepciones
			if (pid == 0) {
				close(child_output);
				if (dup2(parent_input, STDOUT_FILENO) < 0 || dup2(parent_input, STDERR_FILENO) < 0)
					throw std::system_error(errno, std::system_category(), "dup2 failed");
				if (execlp("bash", "bash", "-c", "ls -l", nullptr) == -1)
					throw std::system_error(errno, std::system_category(), "execlp failed");
			} else if (pid > 0) {
				close(parent_input);
				read(child_output, buffer.data(), buffer.size());
				wait(nullptr);
			} else
				throw std::system_error(errno, std::system_category(), "fork() failed");
			fprintf(fd, "200 List okay.\n");
			//fprintf(fd, "%s\n", buffer.data());

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

void ClientConnection::logError(const std::string& error_str) {
	std::cerr << "[ERROR] -- Client " << id() << " -- REASON: " << error_str;
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

	return data_socket;
}

int ClientConnection::cmdPasv(uint16_t& port, char** ip) {
	int sockid = createTcpSocket(LISTEN_MODE, 0, "127.0.0.1");

	if (sockid < 0)
		return -1;

	sockaddr_in address{};
	socklen_t len = sizeof(address);

	// used for the message of entering passive mode
	if (getsockname(sockid, reinterpret_cast<sockaddr*>(&address), &len) < 0) {
		logError(strerror(errno));
		return -1;
	}

	port = address.sin_port;

	char* ip_bin = inet_ntoa(address.sin_addr);
	ip[0] = strtok(ip_bin, ".");
	ip[1] = strtok(nullptr, ".");
	ip[2] = strtok(nullptr, ".");
	ip[3] = strtok(nullptr, ".");

	if ((data_socket = accept(sockid, reinterpret_cast<sockaddr*>(&address), &len)) < 0) {
		logError(strerror(errno));
		return -1;
	}
}
