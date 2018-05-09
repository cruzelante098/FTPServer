//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2ª de grado de Ingeniería Informática
//                       
//              This class processes an FTP transactions.
// 
//****************************************************************************

#include "ClientConnection.h"

ClientConnection::ClientConnection(int socket_id) : control_socket(socket_id) {
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
				std::cerr << "ERROR: PWD: " << strerror(errno) << std::endl;
			} else
				fprintf(fd, "257 \"%s\" is the current directory.\n", cwd);
		} else if (COMMAND("PASS")) {
			fscanf(fd, "%s", arg);
			fprintf(fd, "230 User logged in, proceed.\n");
		} else if (COMMAND("PORT")) { // Fran

		} else if (COMMAND("PASV")) { // Jorge
			fscanf(fd, "%s", arg);
		} else if (COMMAND("CWD")) { // Fran

		} else if (COMMAND("STOR")) { // Jorge

		} else if (COMMAND("SYST")) { // Fran

		} else if (COMMAND("TYPE")) { // Jorge

		} else if (COMMAND("RETR")) { // Fran

		} else if (COMMAND("QUIT")) { // Jorge
			fprintf(fd, "221 Service closing control connection. Goodbye.\n");
			exit = true;
		} else if (COMMAND("LIST")) { // Fran

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
};
