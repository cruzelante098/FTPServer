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
	auto sock = socket_id; // TODO: para que es esto?
	char buffer[MAX_BUFF];
	control_socket = socket_id;

	// TODO: Check the Linux man pages to know what fdopen does.
	fd = fdopen(socket_id, "a+");

	if (fd == nullptr) {
		std::cout << "Connection closed" << std::endl;
		fclose(fd);
		close(control_socket);
		ok = false;
		return;
	}

	ok = true;
	data_socket = -1;
}


ClientConnection::~ClientConnection() {
	fclose(fd);
	close(control_socket);
}


void ClientConnection::stop() {
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

	fprintf(fd, "220 Service ready\n");
	while (!exit) {
		fscanf(fd, "%s", command);
		if (COMMAND("USER")) // Identifica al usuario con su username
		{
			fscanf(fd, "%s", arg);
			fprintf(fd, "331 User name ok, need password\n");
		} else if (COMMAND("PWD")) // Imprime el directorio de trabajo donde actualmente se encuentra el usuario
		{

		} else if (COMMAND("PASS")) //
		{

		} else if (COMMAND("PORT")) {

		} else if (COMMAND("PASV")) {

		} else if (COMMAND("CWD")) {

		} else if (COMMAND("STOR")) {

		} else if (COMMAND("SYST")) {

		} else if (COMMAND("TYPE")) {

		} else if (COMMAND("RETR")) {

		} else if (COMMAND("QUIT")) {

		} else if (COMMAND("LIST")) {

		} else {
			fprintf(fd, "502 Command not implemented.\n");
			fflush(fd);
			printf("Comando : %s %s\n", command, arg);
			printf("Error interno del servidor\n"); // TODO: sustituir por salida de error
		}
	}
	fclose(fd);
};
