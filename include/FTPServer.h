#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <list>

#include "ClientConnection.h"

class FTPServer
{
public:

    explicit FTPServer(int port = 21);

    void run();
    void stop();

private:

    int port;
    int msock;

    std::list<ClientConnection*> connection_list;
};

#endif
