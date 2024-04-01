#include "server.h"


int serverfd, newsockfd, portno;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
int n;
const char *hello = "Hello from client\n";

Server::Server()
{
    portno = 8080;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


}
Server::~Server()
{
    threadClose = true;
    qDebug() << "thread exit: " << endl;;
}

void Server::run()
{
    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    bind(serverfd, (struct sockaddr *) &(serv_addr), sizeof(serv_addr));

    qDebug() << "Wait client: " << endl;

    listen(serverfd,5);

    clilen = sizeof(cli_addr);

    qDebug() << "Client listened: " << endl;

    newsockfd = accept(serverfd, (struct sockaddr *) &(cli_addr), &(clilen));

    qDebug() << "Client connected: " << endl;


    while(!threadClose)
    {

        int valwrite;
//        if ((valread = recv(newsockfd, bufferRx, sizeof(bufferRx), 0)) < 0) {
//            qDebug() << "Receive failed" << endl;
//        }

//        qDebug() << "Recieved: " << bufferRx << endl;

        if(dataInBuffer == BUFF_FULL)
        {
            // Send data to the client
            valwrite = send(newsockfd, bufferTx, 320*240, 0);
            if(valwrite < 0)
            {
                qDebug() << "Send failed" << endl;
            }
            else if (valwrite < (320*240))
            {
                send(newsockfd, (bufferTx+valwrite), ((320*240)-valwrite), 0);
                qDebug() << "Send again remaining data" << endl;
            }
            else
            {
                qDebug() << "Sent OK:" << valwrite << endl;
            }
            msleep(10);
            dataInBuffer = BUFF_EMPTY;
        }

    }
    // Close the socket
    close(newsockfd);
    close(serverfd);
    qDebug() << "Server thread closed" << endl;
}

void Server::thClose()
{
    shutdown(serverfd, SHUT_RD);
    close(newsockfd);
    close(serverfd);
    threadClose = true;
}
