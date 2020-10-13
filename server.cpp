#include <iostream>
#include <cstring>
#include <cstdio>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "./include/SocketBuild.h"

#define MAXBUFFSIZE 100
#define LOCALPORT   "8333"
#define BACKLOG     3

int main(int argc, char* argv[])
{
    int sfd, c_sfd;
    int status;
    int len;
    int optval = 1;

    struct addrinfo hint, *res;
    struct sockaddr_storage c_addr;

    socklen_t addrlen;
    fd_set readfd;

    CreateSocket(hint, res, AI_PASSIVE, NULL, LOCALPORT);

    /*
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags    = AI_PASSIVE;
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    // Set up addrinfo struct for server connection 
    if ((status = getaddrinfo(NULL, LOCALPORT, &hint, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error:%s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Sets up Socket File Descriptor
    if ((sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {   
        perror("Socket:");
        exit(EXIT_FAILURE);
    }
    */
   std::cout << sfd;
    // Prevents "Socket Not Available" Error
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
    {
        perror("setsockopt:");
        exit(EXIT_FAILURE);
    }

    // Binds the server info to file descriptor
    if (bind(sfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sfd);
        perror("Socket Bind:");
        exit(EXIT_FAILURE);
    }

    // Listen on file descriptor. Current queues 3 incomming connections
    if (listen(sfd, BACKLOG) == -1)
    {
        perror("Error on Listen:");
        exit(EXIT_FAILURE);
    }

    // If connection attept succeeds, Establish connection to new file descriptor
    addrlen = sizeof(c_addr);
    if ((c_sfd = accept(sfd, (struct sockaddr*)&c_addr, &addrlen)) < 0)
    {
        close(sfd);
        perror("Error Accepting Connection:");
        exit(EXIT_FAILURE);    
    }

    // Closes original file descriptor to prevent new connections.
    close(sfd);

    while(true)
    {
        char buf[MAXBUFFSIZE];
        int bytes_recv;

        FD_ZERO(&readfd);
        FD_SET(c_sfd, &readfd);
        FD_SET(STDIN_FILENO, &readfd);

        // Helps checks for incoming messages from Client
        select(FD_SETSIZE, &readfd, NULL, NULL, 0);

        bytes_recv = recv(c_sfd, buf, MAXBUFFSIZE, 0);

        if (bytes_recv < 0)
        {
            perror("Message Receive Failed");
            continue;
        }
        else if(bytes_recv == 0)
        {
            printf("Connection has been closed\n");
            break;
        }

        buf[bytes_recv] = 0;
        printf("Client: %s\n", buf);
    }
    close(c_sfd);
}
