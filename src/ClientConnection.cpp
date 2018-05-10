//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2ª de grado de Ingeniería Informática
//                       
//              This class processes an FTP transactions.
// 
//****************************************************************************

#include <err.h>
#include "ClientConnection.h"

ClientConnection::ClientConnection(int socket_id) {
	control_socket = socket_id;
	char buffer[MAX_BUFF];

	// TODO: Check the Linux man pages to know what fdopen does.
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
	//	login();
	// TODO: meter en funciones
	while (!exit) {
		fscanf(fd, "%s", command);
		if (COMMAND("USER")) {

			fscanf(fd, "%s", arg);
			fprintf(fd, "331 User name ok, need password.\n");

		} else if (COMMAND("PWD") || COMMAND("XPWD")) {

			char cwd[200];
			if (!getcwd(cwd, 200)) {
				fprintf(fd, "550 Requested action not taken.\n");
				std::cerr << "[ERROR] PWD: " << strerror(errno) << std::endl;
			} else
				fprintf(fd, "257 \"%s\" is the current directory.\n", cwd);

		} else if (COMMAND("PASS")) {

			fscanf(fd, "%s", arg);
			fprintf(fd, "230 User logged in, proceed.\n");

		} else if (COMMAND("PORT")) {

			std::array<uint32_t, 4> ip;
			std::array<uint16_t, 2> port;
			fscanf(fd, "%d,%d,%d,%d,%hi,%hi", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

			uint32_t ip_bin = ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0];
			uint16_t port_bin = port[1] << 8 | port[0];

			data_socket = connectTCP(ip_bin, port_bin);

			if (data_socket < 0) {
				//fprintf(fd, "425 Can’t open data connection.\n"); // TODO: revisar el mensaje
				fprintf(fd, "550 Requested action not taken.\n");
				std::cerr << "[ERROR] PORT: " << strerror(errno) << std::endl;
			} else {
				fprintf(fd, "200 Command okay.\n");
			}

		} else if (COMMAND("PASV")) { // Jorge

            data_socket = define_socket_TCP(0,"127.0.0.1");
            sockaddr_in address{};
            socklen_t len = sizeof(address);
            if(getsockname(data_socket, reinterpret_cast<sockaddr *>(&address), &len) < 0){
                std::cerr << strerror(errno) << "\n";
            }
            int puerto = address.sin_port;
            char* ip = inet_ntoa(address.sin_addr);
            fprintf(fd, "227 Entering Passive Mode (%s,%s,%s,%s,%i,%i)\n",
                    std::strtok(ip,"."),std::strtok(nullptr,"."),std::strtok(nullptr,"."),std::strtok(nullptr,"."),puerto/256,puerto%256);

		} else if (COMMAND("CWD")) { // Fran

			fprintf(fd, "250 Requested file action okay, completed.\n");

		} else if (COMMAND("STOR")) { // Jorge

            fscanf(fd, "%s", arg);
            

        } else if (COMMAND("SYST")) { // Fran

            fprintf(fd, "215 UNIX Type : L8\n");

        } else if (COMMAND("TYPE")) { // Jorge

		} else if (COMMAND("RETR")) { // Fran

		} else if (COMMAND("QUIT")) { // Jorge

			fprintf(fd, "221 Service closing control connection. Goodbye.\n");
			stop();

		} else if (COMMAND("LIST")) { // Fran

            fprintf(fd, "200 List okay.\n");

		} else {
			fprintf(fd, "502 Command not implemented.\n");
			fflush(fd);
			std::cerr << "Comando recibido: " << command << arg << std::endl;
			std::cerr << "Error interno del servidor" << std::endl;
		}
	}
	fclose(fd);
}

int ClientConnection::id() {
	return control_socket;
}

