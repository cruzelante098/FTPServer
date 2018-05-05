#ifndef ClientConnection_H
#define ClientConnection_H

#include <pthread.h>

#include <cstdio>
#include <cstdint>

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <clocale>
#include <langinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <iostream>
#include <dirent.h>

#include "common.h"

#define COMMAND(cmd) strcmp(command, cmd)==0

const int MAX_BUFF=1000;

class ClientConnection {
public:
    explicit ClientConnection(int s);
    ~ClientConnection();
    
    void WaitForRequests();
    void stop();

private:  
   bool ok;  // This variable is flag that avois that the
	     // server listens if initialization errors occured.
    
    FILE *fd;	 // C file descriptor. We use it to buffer the
		 // control connection of the socket and allows to
		 // manage it as a C file usign fprintf, fscanf, etc.

    char command[MAX_BUFF];  // Buffer for saving the command.
    char arg[MAX_BUFF];      // Buffer for saving the arguments. 
    
    int data_socket;         // Data socket descriptor;
    int control_socket;      // Control socket descriptor;
    
    bool parar;
};

#endif
