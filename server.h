#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <QThread>
#include <QTime>
#include <QDebug>
#include "process.h"

#define BUFF_EMPTY    0
#define BUFF_FULL     1

class Server : public QThread
{
private:
    bool threadClose;

public:

    Server();
    ~Server();
    void run();
    void thClose();

    uchar bufferTx[320*240];
    int dataInBuffer;
};

#endif // SERVER_H
