#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;
int port;
#define MAX 100

void socketMaker(int sd)
{
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("Eroare socket!\n");
}

void serverConnection(int sd, struct sockaddr_in server)
{
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        perror("Eroare connect[client]");
}

int main(int argc, char *argv[])
{
    int socketDesc;            // descriptor socket
    struct sockaddr_in server; // struct pentru connect

    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    }

    port = atoi(argv[2]);

    socketMaker(socketDesc);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    serverConnection(socketDesc, server);

    char command[MAX];

    while (1)
    {
        printf("Welcome");
        bzero(command, sizeof(command));
        scanf("%s", command);

        write(socketDesc, command, strlen(command));
    }
}